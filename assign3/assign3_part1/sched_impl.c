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

	sem_post(&(sched_queue_info->empty_queue_lock));
}
static void leave_sched_queue(thread_info_t *info)
{
	list_remove_elem(info->sched_queue_info->list_queue, info->thread_node);

	
}
static void wait_for_cpu_fifo(thread_info_t *info)
{

	sem_wait(&info->thread_exec_lock);
	sem_wait(&info->sched_queue_info->cpu_lock);
}
static void wait_for_cpu_rr(thread_info_t *info)
{
	// printf("%lu waiting for thread_exec_lock\n", getThreadId(info));
	int* val = (int*)malloc(sizeof val);
	sem_t* sem = &info->sched_queue_info->cpu_lock;
	sem_getvalue(sem, val);
	// printf("cpu_lock = %d\n", *val);
	sem_wait(&info->thread_exec_lock);
	// printf("%lu waiting for cpu_lock\n", getThreadId(info));
	sem_wait(&info->sched_queue_info->cpu_lock);
	// printf("%lu finish waiting\n", getThreadId(info));
}
static void release_cpu(thread_info_t *info)
{	
	sem_post(&info->sched_queue_info->cpu_lock);
	sem_post(&info->thread_exec_lock);
	// printf("%lu release\n", getThreadId(info));

}

/*...More functions go here...*/
static void init_sched_queue(sched_queue_t *queue, int queue_size)
{
	/*...Code goes here...*/
	queue->list_queue = (list_t*) malloc(sizeof(list_t));
	list_init(queue->list_queue);
	queue->size = queue_size;
	sem_init(&queue->cpu_lock, 0, 1);
	sem_init(&queue->empty_queue_lock, 0 , 0);


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
	// printf("Waking up worker %lu\n", (unsigned long) info->thread_id);
	sem_post(&info->thread_exec_lock);
	sem_post(&info->sched_queue_info->cpu_lock);

}
static void wait_for_worker(sched_queue_t *queue)
{

	// thread_info_t* current_thread = (thread_info_t*) queue->exec_thread_node->datum;
	// printf("Wait for worker\n");
	
	sem_wait(&queue->cpu_lock);
	// printf("Regain control\n");
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

	list_elem_t* temp_head = list_get_head(queue->list_queue);
	if(temp_head == NULL)
		return NULL;
	if(temp_head->next == NULL)
		return (thread_info_t *)(temp_head->datum);
	list_remove_elem(queue->list_queue, temp_head);
	list_insert_tail(queue->list_queue, temp_head);

	temp_head = list_get_head(queue->list_queue);
	if(temp_head == NULL)
		return NULL;
	thread_info_t * next_worker_fifo = (thread_info_t *)(temp_head->datum);

	return next_worker_fifo;

}
static void wait_for_queue(sched_queue_t *queue)
{
	// printf("Waiting for empty queue\n");
	sem_wait(&queue->empty_queue_lock);
	// printf("Have something in queue\n");
}

/*...More functions go here...*/

/* You need to statically initialize these structures: */
sched_impl_t sched_fifo = {
	{ init_thread_info, destroy_thread_info, enter_sched_queue, leave_sched_queue, wait_for_cpu_fifo, release_cpu/*, ...etc... */ }, 
	{ init_sched_queue, destroy_sched_queue, wake_up_worker, wait_for_worker, next_worker_fifo, wait_for_queue/*, ...etc... */ } },
sched_rr = {
	{ init_thread_info, destroy_thread_info, enter_sched_queue, leave_sched_queue, wait_for_cpu_rr, release_cpu, /*...etc...*/  }, 
	{ init_sched_queue, destroy_sched_queue, wake_up_worker, wait_for_worker, next_worker_rr, wait_for_queue/*, ...etc... */ } };
