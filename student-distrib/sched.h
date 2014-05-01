#ifndef _SCHED_H
#define _SCHED_H

#include "types.h" 
#include "queue.h"
#include "proc.h"

/* Declare Queues used for scheduling: 
 *	Queue_1 starts out as active queue
 *	Queue_2 starts out as expired queue
 */
 
typedef CIRC_BUF_TYPE(uint32_t, MAX_PROCESSES + 1) sched_queue_t;

extern sched_queue_t* active_queue;
extern sched_queue_t* expired_queue;

/* Contains information the scheduler uses
 * 	that is accessed or changed globally
 * 	**Note: Contains flags that are modified by an interrupt
 */
typedef struct sched_flags{
	volatile uint8_t isZombie;
	volatile uint8_t relaunch;
} sched_flags_t;

extern sched_flags_t sched_flags;

void init_sched(void);
uint8_t push_to_active(uint32_t pid);
uint8_t push_to_expired(uint32_t pid);
uint8_t remove_active_from_sched(void);
uint8_t active_to_expired(void);
void swap_queues(void);

#endif /* _SCHED_H */
