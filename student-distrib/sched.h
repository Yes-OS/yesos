#ifndef _SCHED_H
#define _SCHED_H

#include "types.h" 
#include "queue.h"
#include "proc.h"

/*Declare Queues used for scheduling: 
 *	Queue_1 starts out as active queue
 *	Queue_2 starts out as expired queue
 */
 
typedef CIRC_BUF_TYPE(uint32_t, MAX_PROCESSES + 1) sched_queue_t;

extern sched_queue_t* active_queue;
extern sched_queue_t* expired_queue;

/*Initialize Queues for Scheduling*/
void init_sched(void);

/*New process started. Add it to the currently active queue*/
uint32_t add_proc_to_sched(uint32_t pid);

/*Process finished. Remove it from the sched completely*/
uint32_t remove_active_from_sched(void);

/*Move process from active queue to expired queue*/
uint32_t active_to_expired(uint32_t pid);

/*Swap the active queue with the expired queue when active queue is empty*/
void swap_queues(void);

#endif /* _SCHED_H */
