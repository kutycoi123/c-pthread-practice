#ifndef	__SCHED_IMPL__H__
#define	__SCHED_IMPL__H__

#include "list.h"

struct thread_info {
	/*...Fill this in...*/
	sched_queue_t* sched_queue_info;
	list_elem_t* thread_node;


};

struct sched_queue {
	/*...Fill this in...*/
	int size;
	list_t list_based_queue;
};

#endif /* __SCHED_IMPL__H__ */
