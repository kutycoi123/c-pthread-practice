#include <stdlib.h>
#include "scheduler.h"
#include "sched_impl.h"
#include "pthread.h"
#include "semaphore.h"
#include "list.h"
/* Fill in your scheduler implementation code below: */

static void init_thread_info(thread_info_t *info, sched_queue_t *queue)
{
	/*...Code goes here...*/
	info->sched_queue = queue;
	sem_init(&info->thread_lock, 0, 0);
}

static void destroy_thread_info(thread_info_t *info)
{
	/*...Code goes here...*/
	info->sched_queue = NULL;
	// free(info->worker_node);
	info->worker_node = NULL;
}

static void enter_sched_queue(thread_info_t *info)
{
	// Wait until the queue has enough slots
	sem_wait(&info->sched_queue->queue_size_lock);
	sched_queue_t* sched_queue = info->sched_queue;

	// Lock to operate on queues
	pthread_mutex_lock(&sched_queue->queue_lock);

	list_elem_t* elem = (list_elem_t*) malloc(sizeof(list_elem_t));

	elem->datum = (void*) info;

	list_insert_tail(sched_queue->worker_queue, elem);

	info->worker_node = list_get_tail(sched_queue->worker_queue);

	// Release locks
	pthread_mutex_unlock(&sched_queue->queue_lock);


}
static void leave_sched_queue(thread_info_t *info)
{

	pthread_mutex_lock(&info->sched_queue->queue_lock);


	list_remove_elem(info->sched_queue->worker_queue, info->worker_node);

	info->sched_queue->current_worker_node = NULL;

	// Make slots for other workers
	sem_post(&info->sched_queue->queue_size_lock);

	pthread_mutex_unlock(&info->sched_queue->queue_lock);

	
}
static void wait_for_cpu(thread_info_t *info)
{
	// Wait until the thread is unlocked
	sem_wait(&info->thread_lock);

}

static void release_cpu(thread_info_t *info)
{	
	// Release cpu_lock for other workers or sheduler to use
	sem_post(&info->sched_queue->cpu_lock);


}

/*...More functions go here...*/
static void init_sched_queue(sched_queue_t *queue, int queue_size)
{
	/*...Code goes here...*/
	queue->worker_queue = (list_t*) malloc(sizeof(list_t));
	list_init(queue->worker_queue);
	queue->size = queue_size;
	queue->current_worker_node = NULL;
	sem_init(&queue->cpu_lock, 0, 0);
	sem_init(&queue->queue_size_lock, 0, queue_size);
	pthread_mutex_init(&queue->queue_lock, NULL);

}

static void destroy_sched_queue(sched_queue_t *queue)
{
	/*...Code goes here...*/
	list_elem_t *curr;
	while( (curr = list_get_head(queue->worker_queue)) != NULL){
		list_remove_elem(queue->worker_queue, curr);
		free(curr);
	}
}

static void wake_up_worker(thread_info_t *info)
{
	// Unlock thread
	sem_post(&info->thread_lock);

}
static void wait_for_worker(sched_queue_t *queue)
{
	// Wait until worker release cpu_lock
	sem_wait(&queue->cpu_lock);
}
/* For fifo policy, always pick the worker sitting at the head of queue*/
static thread_info_t * next_worker_fifo(sched_queue_t *queue)
{
	pthread_mutex_lock(&queue->queue_lock);

	list_elem_t* head = list_get_head(queue->worker_queue);
	if(head == NULL){
		pthread_mutex_unlock(&queue->queue_lock);
		return NULL;
	}
	thread_info_t * next_worker_fifo = (thread_info_t *)(head->datum);
	pthread_mutex_unlock(&queue->queue_lock);
	
	return next_worker_fifo;
}
/* For rr policy, remove the current worker, move it to the end of queue and pick 
the worker sitting at the head of queue as next worker*/
static thread_info_t * next_worker_rr(sched_queue_t *queue)
{

	pthread_mutex_lock(&queue->queue_lock);

	if(queue->current_worker_node != NULL){
		list_remove_elem(queue->worker_queue, queue->current_worker_node);
		list_insert_tail(queue->worker_queue, queue->current_worker_node);
	}

	queue->current_worker_node = list_get_head(queue->worker_queue);

	if(queue->current_worker_node == NULL){
		pthread_mutex_unlock(&queue->queue_lock);
		return NULL;
	}
	thread_info_t * next_worker_fifo = (thread_info_t *)(queue->current_worker_node->datum);

	pthread_mutex_unlock(&queue->queue_lock);

	return next_worker_fifo;


}

static void wait_for_queue(sched_queue_t *queue)
{
	while(!list_size(queue->worker_queue)){
		sched_yield();
	}

}



/* You need to statically initialize these structures: */
sched_impl_t sched_fifo = {
	{ init_thread_info, destroy_thread_info, enter_sched_queue, leave_sched_queue, wait_for_cpu, release_cpu/*, ...etc... */ }, 
	{ init_sched_queue, destroy_sched_queue, wake_up_worker, wait_for_worker, next_worker_fifo, wait_for_queue/*, ...etc... */ } },
sched_rr = {
	{ init_thread_info, destroy_thread_info, enter_sched_queue, leave_sched_queue, wait_for_cpu, release_cpu, /*...etc...*/  }, 
	{ init_sched_queue, destroy_sched_queue, wake_up_worker, wait_for_worker, next_worker_rr, wait_for_queue/*, ...etc... */ } };
