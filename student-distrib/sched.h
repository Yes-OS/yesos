/* sched.h, process header file
 * vim:ts=4 sw=4 noexpandtab
 */
#ifndef _SCHED_H
#define _SCHED_H

#include "types.h"
#include "queue.h"
#include "proc.h"

#ifndef ASM

/****************************************
 *              Data Types              *
 ****************************************/

/* Create the schedule queue buffers of a set size depending
 *  on the number of process we have designed for
 */
typedef CIRC_BUF_TYPE(uint32_t, MAX_PROCESSES + 1) sched_queue_t;

/* Contains information the scheduler uses
 *  that is accessed or changed globally
 *  **Note: Contains flags that are modified by an interrupt
 */
typedef struct sched_flags{
	volatile uint8_t isZombie;
	volatile uint8_t relaunch;
} sched_flags_t;


/****************************************
 *           Global Variables           *
 ****************************************/

extern sched_queue_t* active_queue;
extern sched_queue_t* expired_queue;

extern sched_flags_t sched_flags;


/****************************************
 *         Function Declarations        *
 ****************************************/

/* Initialize the scheduler and its data */
void init_sched(void);

/* Pushes a PID to the active queue */
uint8_t push_to_active(uint32_t pid);

/* Pushes a PID to the expired queue */
uint8_t push_to_expired(uint32_t pid);

/* Sets the PID status of the end process in the active queue for removal */
uint8_t remove_active_from_sched(void);

/* Moves the latest active process from the active queue to the expired queue */
uint8_t active_to_expired(void);

/* Swaps pointers for the active and expired queues */
void swap_queues(void);

/* Is the main function that runs the scheduler. Includes context switch helper */
void scheduler(registers_t* regs);


#endif /* ASM */
#endif /* _SCHED_H */

