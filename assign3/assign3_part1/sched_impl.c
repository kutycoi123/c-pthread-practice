#include <stdlib.h>
#include "scheduler.h"
#include "sched_impl.h"
#include "pthread.h"
#include "list.h"
/* Fill in your scheduler implementation code below: */

static void init_thread_info(thread_info_t *info, sched_queue_t *queue)
{
	/*...Code goes here...*/
	info->sched_queue_info = queue;
}

static void destroy_thread_info(thread_info_t *info)
{
	/*...Code goes here...*/
	info->sched_queue_info = NULL;
	info->thread_node = NULL;
}

static void enter_sched_queue(thread_info_t *info)
{
	sched_queue_t* sched_queue_info = info->sched_queue_info;
	int maxSize = sched_queue_info->size;
	//int currentSize = list_size(&(info->sched_queue_info->list_based_queue));
	while(list_size(&(sched_queue_info->list_based_queue)) >= maxSize);

	list_elem_t* elem = (list_elem_t*) malloc(sizeof(list_elem_t));

	elem->datum = (void*) info;
	
	list_insert_tail(&(sched_queue_info->list_based_queue), elem);
	info->thread_node = list_get_tail(&(sched_queue_info->list_based_queue));
}
static void leave_sched_queue(thread_info_t *info)
{
	list_remove_elem(&(info->sched_queue_info->list_based_queue), info->thread_node);
	
}
static void wait_for_cpu(thread_info_t *info)
{

}
static void release_cpu(thread_info_t *info)
{

}

/*...More functions go here...*/
static void init_sched_queue(sched_queue_t *queue, int queue_size)
{
	/*...Code goes here...*/
	list_init(&(queue->list_based_queue));
	queue->size = queue_size;
	//queue->next_worker_node = list_get_head(&(queue->list_based_queue));
}

static void destroy_sched_queue(sched_queue_t *queue)
{
	/*...Code goes here...*/
	list_elem_t *curr;
	while( (curr = list_get_head(&(queue->list_based_queue))) != NULL){
		list_remove_elem(&(queue->list_based_queue), curr);
		free(curr);
	}
}

static void wake_up_worker(thread_info_t *info)
{

}
static void wait_for_worker(sched_queue_t *queue)
{
	//while(list_get_head(&(queue->list_based_queue)) == NULL);
}
static thread_info_t * next_worker(sched_queue_t *queue)
{
	list_elem_t* head = list_get_head(&(queue->list_based_queue));
	if(head == NULL)
		return NULL;
	thread_info_t * next_worker_fifo = (thread_info_t *)(head->datum);
	
	return next_worker_fifo;
}
static void wait_for_queue(sched_queue_t *queue)
{
	while(list_get_head(&(queue->list_based_queue)) == NULL){
		// printf("Stuck\n");
		sched_yield();

	}
}

/*...More functions go here...*/

/* You need to statically initialize these structures: */
sched_impl_t sched_fifo = {
	{ init_thread_info, destroy_thread_info, enter_sched_queue, leave_sched_queue, wait_for_cpu, release_cpu/*, ...etc... */ }, 
	{ init_sched_queue, destroy_sched_queue, wake_up_worker, wait_for_worker, next_worker, wait_for_queue/*, ...etc... */ } },
sched_rr = {
	{ init_thread_info, destroy_thread_info, enter_sched_queue, leave_sched_queue, wait_for_cpu, release_cpu, /*...etc...*/  }, 
	{ init_sched_queue, destroy_sched_queue, wake_up_worker, wait_for_worker, next_worker, wait_for_queue/*, ...etc... */ } };
