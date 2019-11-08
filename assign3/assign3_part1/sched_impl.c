#include <stdlib.h>
#include "scheduler.h"
#include "sched_impl.h"
#include "pthread.h"
#include "semaphore.h"
#include "list.h"
/* Fill in your scheduler implementation code below: */


static unsigned long getThreadId(thread_info_t* t){
	return (unsigned long)t->thread_id;
}
static void _print(list_elem_t* t){
	// printf("Hello\n");
	thread_info_t* _t = (thread_info_t*) t->datum;
	printf("\t%lu\n", getThreadId(_t));
}
static void init_thread_info(thread_info_t *info, sched_queue_t *queue)
{
	/*...Code goes here...*/
	info->sched_queue_info = queue;
	sem_init(&info->sem_lock_thread, 0 , 0);
	sem_init(&info->thread_exec_lock, 0, 0);
}

static void destroy_thread_info(thread_info_t *info)
{
	/*...Code goes here...*/
	info->sched_queue_info = NULL;
	// free(info->thread_node);
	info->thread_node = NULL;
}

static void enter_sched_queue(thread_info_t *info)
{

	sem_wait(&info->sched_queue_info->queue_sem);
	sched_queue_t* sched_queue_info = info->sched_queue_info;

	pthread_mutex_lock(&sched_queue_info->queue_lock);

	list_elem_t* elem = (list_elem_t*) malloc(sizeof(list_elem_t));

	elem->datum = (void*) info;

	list_insert_tail(sched_queue_info->list_queue, elem);
	info->thread_node = list_get_tail(sched_queue_info->list_queue);

	pthread_mutex_unlock(&sched_queue_info->queue_lock);


	sem_post(&(sched_queue_info->empty_queue_lock));

}
static void leave_sched_queue(thread_info_t *info)
{

	pthread_mutex_lock(&info->sched_queue_info->queue_lock);


	list_remove_elem(info->sched_queue_info->list_queue, info->thread_node);

	info->sched_queue_info->exec_thread_node = NULL;


	list_foreach(info->sched_queue_info->list_queue, _print);

	sem_post(&info->sched_queue_info->queue_sem);

	pthread_mutex_unlock(&info->sched_queue_info->queue_lock);




	
}
static void wait_for_cpu_fifo(thread_info_t *info)
{

	sem_wait(&info->thread_exec_lock);

}
static void wait_for_cpu_rr(thread_info_t *info)
{
	// printf("%lu wait for cpu\n", getThreadId(info));
	sem_wait(&info->thread_exec_lock);


}
static void release_cpu(thread_info_t *info)
{	
	// printf("%lu releasing\n", getThreadId(info));
	sem_post(&info->sched_queue_info->cpu_lock);


}

/*...More functions go here...*/
static void init_sched_queue(sched_queue_t *queue, int queue_size)
{
	/*...Code goes here...*/
	queue->list_queue = (list_t*) malloc(sizeof(list_t));
	list_init(queue->list_queue);
	queue->size = queue_size;
	queue->exec_thread_node = NULL;
	sem_init(&queue->cpu_lock, 0, 0);
	sem_init(&queue->empty_queue_lock, 0 , 0);
	sem_init(&queue->queue_sem, 0, queue_size);
	pthread_mutex_init(&queue->queue_lock, NULL);


}

static void destroy_sched_queue(sched_queue_t *queue)
{
	/*...Code goes here...*/
	list_elem_t *curr;
	while( (curr = list_get_head(queue->list_queue)) != NULL){
		list_remove_elem(queue->list_queue, curr);
		free(curr);
	}
}

static void wake_up_worker(thread_info_t *info)
{
	// info->sched_queue_info->exec_thread_node = info->thread_node;
	// printf("%lu woke up\n", getThreadId(info));
	sem_post(&info->thread_exec_lock);

}
static void wait_for_worker(sched_queue_t *queue)
{

	// thread_info_t* current_thread = (thread_info_t*) queue->exec_thread_node->datum;
	// printf("Scheduler waiting\n");
	sem_wait(&queue->cpu_lock);
	// printf("SCheduler regain control\n");
}

static thread_info_t * next_worker_fifo(sched_queue_t *queue)
{
	pthread_mutex_lock(&queue->queue_lock);

	list_elem_t* head = list_get_head(queue->list_queue);
	if(head == NULL){
		pthread_mutex_unlock(&queue->queue_lock);
		return NULL;
	}
	thread_info_t * next_worker_fifo = (thread_info_t *)(head->datum);
	pthread_mutex_unlock(&queue->queue_lock);
	
	return next_worker_fifo;
}

static thread_info_t * next_worker_rr(sched_queue_t *queue)
{

	// pthread_mutex_lock(&queue->queue_lock);

	// list_elem_t* temp_head = list_get_head(queue->list_queue);

	// if(temp_head == NULL){
	// pthread_mutex_unlock(&queue->queue_lock);

	// 	return NULL;
	// }
	// if(temp_head->next == NULL){
	// 	pthread_mutex_unlock(&queue->queue_lock);

	// 	return (thread_info_t *)(temp_head->datum);
	// }


	// list_remove_elem(queue->list_queue, temp_head);
	// list_insert_tail(queue->list_queue, temp_head);

	// temp_head = list_get_head(queue->list_queue);
	// if(temp_head == NULL){
	// 	pthread_mutex_unlock(&queue->queue_lock);

	// 	return NULL;
	// }
	// thread_info_t * next_worker_fifo = (thread_info_t *)(temp_head->datum);

	// pthread_mutex_unlock(&queue->queue_lock);

	// return next_worker_fifo;

	/*Version 2*/

	pthread_mutex_lock(&queue->queue_lock);
	// list_foreach(queue->list_queue, _print);

	if(queue->exec_thread_node != NULL){
		list_remove_elem(queue->list_queue, queue->exec_thread_node);
		list_insert_tail(queue->list_queue, queue->exec_thread_node);
	}

	queue->exec_thread_node = list_get_head(queue->list_queue);

	if(queue->exec_thread_node == NULL){
		pthread_mutex_unlock(&queue->queue_lock);
		return NULL;
	}
	thread_info_t * next_worker_fifo = (thread_info_t *)(queue->exec_thread_node->datum);

	pthread_mutex_unlock(&queue->queue_lock);
	// printf("Have next worker is %lu\n", getThreadId(next_worker_fifo));
	return next_worker_fifo;


}
static void wait_for_queue(sched_queue_t *queue)
{
	// printf("Waiting for empty queue\n");
	sem_wait(&queue->empty_queue_lock);
	// printf("Have something in queue\n");
	// while(!list_size(&queue->list_queue)){
	// 	sched_yield();
	// }
}

/*...More functions go here...*/

/* You need to statically initialize these structures: */
sched_impl_t sched_fifo = {
	{ init_thread_info, destroy_thread_info, enter_sched_queue, leave_sched_queue, wait_for_cpu_fifo, release_cpu/*, ...etc... */ }, 
	{ init_sched_queue, destroy_sched_queue, wake_up_worker, wait_for_worker, next_worker_fifo, wait_for_queue/*, ...etc... */ } },
sched_rr = {
	{ init_thread_info, destroy_thread_info, enter_sched_queue, leave_sched_queue, wait_for_cpu_rr, release_cpu, /*...etc...*/  }, 
	{ init_sched_queue, destroy_sched_queue, wake_up_worker, wait_for_worker, next_worker_rr, wait_for_queue/*, ...etc... */ } };
