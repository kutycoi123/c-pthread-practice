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
	info->sched_queue_info = queue;
	sem_init(&info->sem_lock_thread, 0 , 0);
	sem_init(&info->thread_exec, 0, 0);
}

static void destroy_thread_info(thread_info_t *info)
{
	/*...Code goes here...*/
	info->sched_queue_info = NULL;
	free(info->thread_node);
	info->thread_node = NULL;
}

static void enter_sched_queue(thread_info_t *info)
{
	sched_queue_t* sched_queue_info = info->sched_queue_info;
	int maxSize = sched_queue_info->size;
	//int currentSize = list_size(&(info->sched_queue_info->list_queue));
	while(list_size(sched_queue_info->list_queue) >= maxSize);

	list_elem_t* elem = (list_elem_t*) malloc(sizeof(list_elem_t));

	elem->datum = (void*) info;
	
	list_insert_tail(sched_queue_info->list_queue, elem);
	info->thread_node = list_get_tail(sched_queue_info->list_queue);

	sem_post(&(sched_queue_info->empty_lock));
}
static void leave_sched_queue(thread_info_t *info)
{
	list_remove_elem(info->sched_queue_info->list_queue, info->thread_node);
	sem_post(&info->thread_exec);
	
}
static void wait_for_cpu_fifo(thread_info_t *info)
{
	//Wait until the thread_node is the head of fifo queue
	// list_elem_t* thread_node_ = info->thread_node;
	// list_elem_t* exec_thread_node_ = info->sched_queue_info->exec_thread_node;

	// thread_info_t* thread_info = (thread_info_t*) thread_node_->datum;
	// thread_info_t* exec_thread_info;
	// if(exec_thread_node_ != NULL){

	// 	exec_thread_info = (thread_info_t*) exec_thread_node_->datum;
	// }
	// while(thread_node_ != exec_thread_node_){
	// 	sched_yield();
	// 	// printf("\n");
	// 	// printf("%lu: %lu\n",(unsigned long)thread_info->thread_id, (unsigned long)exec_thread_info->thread_id);
	// }
	// sem_wait(&info->sem_lock_thread);
	// sem_post(&info->sem_lock_thread);
}
static void wait_for_cpu_rr(thread_info_t *info)
{
	//Wait until the thread_node is the head of fifo queue
	// while(info->thread_node != list_get_head(info->sched_queue_info->list_queue)){
	// 	sched_yield();
	// }
}
static void release_cpu(thread_info_t *info)
{

}

/*...More functions go here...*/
static void init_sched_queue(sched_queue_t *queue, int queue_size)
{
	/*...Code goes here...*/
	queue->list_queue = (list_t*) malloc(sizeof(list_t));
	list_init(queue->list_queue);
	queue->size = queue_size;
	sem_init(&queue->sem_lock_queue, 0, 1);
	sem_init(&queue->empty_lock, 0 , 0);

	//queue->next_worker_node = list_get_head(&(queue->list_queue));
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
	info->sched_queue_info->exec_thread_node = info->thread_node;
	// sem_post(&info->sem_lock_thread);
}
static void wait_for_worker(sched_queue_t *queue)
{
	//while(list_get_head(&(queue->list_queue)) == NULL);
	// sem_wait(&queue->sem_lock_queue);
	// sem_post(&queue->sem_lock_queue);
	thread_info_t* current_thread = (thread_info_t*) queue->exec_thread_node->datum;
	sem_wait(&current_thread->thread_exec);
}

static thread_info_t * next_worker_fifo(sched_queue_t *queue)
{
	list_elem_t* head = list_get_head(queue->list_queue);
	if(head == NULL)
		return NULL;
	thread_info_t * next_worker_fifo = (thread_info_t *)(head->datum);
	
	return next_worker_fifo;
}

static thread_info_t * next_worker_rr(sched_queue_t *queue)
{
	list_elem_t* head = list_get_head(queue->list_queue);
	if(head == NULL)
		return NULL;
	thread_info_t * next_worker_fifo = (thread_info_t *)(head->datum);
	
	return next_worker_fifo;
}
static void wait_for_queue(sched_queue_t *queue)
{
	// while(list_get_head(queue->list_queue) == NULL){
	// 	// printf("Stuck\n");
	// 	sched_yield();

	// }
	sem_wait(&queue->empty_lock);
}

/*...More functions go here...*/

/* You need to statically initialize these structures: */
sched_impl_t sched_fifo = {
	{ init_thread_info, destroy_thread_info, enter_sched_queue, leave_sched_queue, wait_for_cpu_fifo, release_cpu/*, ...etc... */ }, 
	{ init_sched_queue, destroy_sched_queue, wake_up_worker, wait_for_worker, next_worker_fifo, wait_for_queue/*, ...etc... */ } },
sched_rr = {
	{ init_thread_info, destroy_thread_info, enter_sched_queue, leave_sched_queue, wait_for_cpu_rr, release_cpu, /*...etc...*/  }, 
	{ init_sched_queue, destroy_sched_queue, wake_up_worker, wait_for_worker, next_worker_rr, wait_for_queue/*, ...etc... */ } };
