#ifndef	__SCHED_IMPL__H__
#define	__SCHED_IMPL__H__

#include "list.h"
#include "semaphore.h"
struct thread_info {
	/*...Fill this in...*/
	sched_queue_t* sched_queue; /*Point to the scheduling queue that this thread is put in*/

	list_elem_t* worker_node; /*Point to the node of thread's worker*/

	sem_t thread_lock; /* lock if thread is waiting for CPU*/

	pthread_t thread_id; /*Thread id - for debugging purpose*/

};

struct sched_queue {
	/*...Fill this in...*/
	int size;

	list_t *worker_queue; /* Scheduler queue*/

	list_elem_t* current_worker_node;/*Point to the node of currently executed worker*/

	sem_t cpu_lock; /* lock for either scheduler or worker to use CPU*/

	sem_t queue_size_lock; /* lock if queue is full */

	pthread_mutex_t	queue_lock; /* lock for queue operations */
};

#endif /* __SCHED_IMPL__H__ */
