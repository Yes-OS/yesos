/* pit.c, implement PIT interrupt functions
 * vim:ts=4 sw=4 noexpandtab
 */

#include "pit.h"
#include "lib.h"
#include "types.h"
#include "i8259.h"
#include "isr.h"
#include "sched.h"
#include "syscall.h"
#include "term.h"

#define reboot 0

/* Static helper function */
static void pit_set_count(void);

/* Initialization of the PIT to sent interrupts as desired
 */
void pit_init(void)
{
	/* Set Initialization Command Number */
	outb(INIT_CMD, MODE_CMD_PORT);

	/* Set PIT counter value*/
	pit_set_count();
}

/* Interrupt handler for the PIT
 *  Resets the PIT for the specified time slot
 *  Calls the scheduler to handle processes
 *
 *  Inputs: regs - context of the process that was interrupted
 */
void pit_handle_interrupt(registers_t* regs)
{
	/* reset PIT counter */
	pit_set_count();

	/* Update scheduling queues and context switch */
	scheduler(regs);
}

/* Set the count value for the PIT
 * This is where we decide how long to make the scheduler time slots
 */
void pit_set_count(void)
{
	/* Set the frequency to the desired time-slice for scheduling
	 * Sends LO and HI bytes by convention
	 */
	outb(SCHED_FREQ_LO, CHAN0_PORT);
	outb(SCHED_FREQ_HI, CHAN0_PORT);
}

