/* sched.h, process header file
 * vim:ts=4 sw=4 noexpandtab
 */

#include "types.h" 
#include "queue.h"
#include "proc.h"
#include "sched.h"

DECLARE_CIRC_BUF(uint32_t, SCHED_QUEUE_1, MAX_PROCESSES + 1);
DECLARE_CIRC_BUF(uint32_t, SCHED_QUEUE_2, MAX_PROCESSES + 1);

sched_queue_t* active_queue;
sched_queue_t* expired_queue;

void init_sched(void)
{
	CIRC_BUF_INIT(SCHED_QUEUE_1);
	CIRC_BUF_INIT(SCHED_QUEUE_2);
	
	active_queue = (sched_queue_t*)&SCHED_QUEUE_1;
	expired_queue = (sched_queue_t*)&SCHED_QUEUE_2;
}

/*New process started. Add it to the currently active queue*/
uint32_t add_proc_to_sched(uint32_t pid)
{
	uint32_t ok;
	
	CIRC_BUF_PUSH(*active_queue, pid, ok); 
	
	if(!ok) {
		return -1;
	}
	
	return 0;
}

/* Process finished. 
 * Remove it from the scheduler completely
 */
uint32_t remove_proc_from_sched(void)
{
	uint32_t ok, temp;
	
	CIRC_BUF_POP(*active_queue, temp, ok);
	
	if(!ok) {
		return -1;
	}

	return 0;
}

/* Push process from active queue to expired queue. 
 * 
 * Returns 0 on success, -1 on failure
 */
uint32_t active_to_expired(uint32_t pid)
{
	uint32_t ok, temp;
	
	CIRC_BUF_POP(*active_queue, temp, ok);
	
	/* If popping from an empty queue */
	if(!ok) {
		return -1;
	}

	CIRC_BUF_PUSH(*expired_queue, temp, ok); 
	
	/* If pushing to a full queue */
	if(!ok) {
		return -1;
	}
	
	return 0;
}

/*Swap the active queue with the expired queue when active queue is empty*/
void swap_queues(void)
{
	sched_queue_t* temp = active_queue;
	active_queue = expired_queue;
	expired_queue = temp;
}
