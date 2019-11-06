#ifndef	__SCHED_IMPL__H__
#define	__SCHED_IMPL__H__

#include "list.h"
#include "semaphore.h"
struct thread_info {
	/*...Fill this in...*/
	sched_queue_t* sched_queue_info;
	list_elem_t* thread_node;
	sem_t sem_lock_thread;
	sem_t sem_lock_thread_done;
	pthread_t thread_id;

};

struct sched_queue {
	/*...Fill this in...*/
	int size;
	list_t *list_queue;
	list_elem_t* exec_thread_node;
	sem_t sem_lock_queue;
};

#endif /* __SCHED_IMPL__H__ */
