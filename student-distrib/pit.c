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

/* Static helper functions */
static void pit_set_count(void);
static void context_switch(void);

/* Initialization of the PIT */
void pit_init(void)
{
	/* Set Initialization Command Number */
	outb(INIT_CMD, MODE_CMD_PORT);

	/* Set PIT counter value*/
	pit_set_count();
}

/* Interrupt handler for the PIT*/
void pit_handle_interrupt(registers_t* regs)
{
	uint8_t ok = 0; /* error flag for sched funcs */

	/* context switching */
	context_switch(regs);

	/* Update Scheduling queues */
	if (sched_flags.isZombie){
		/* Remove and discard from active queue */
		ok = remove_active_from_sched();
		sched_flags.isZombie = 0;
	}
	else {
		ok = active_to_expired();
	}

#if 0
	/* No, you can't */
	if (sched_flags.relaunch) {
		/* When top shell is exited, must reboot */
		sched_flags.relaunch = 0;
		sys_exec((uint8_t*)"shell"); /* CAN I DO DIS? */
	}
#endif

	if (CIRC_BUF_EMPTY(*active_queue)) {
		swap_queues();
	}

	/* reset PIT counter */
	pit_set_count();

}

/* set the count value for the PIT */
void pit_set_count(void)
{
	/* Set the frequency to the desired time-slice for scheduling
	 * Sends LO and HI bytes by convention
	 */
	outb(SCHED_FREQ_LO, CHAN0_PORT);
	outb(SCHED_FREQ_HI, CHAN0_PORT);
}

/* Filler right now
 */

/*
 void context_switch(void)
{
	This won't actually work as we expect it to 
	int x = screen_x;
	int y = screen_y;
	screen_x = NUM_COLS-8;
	screen_y = NUM_ROWS-1;
	update_cursor();

	printf("TERM: %d", terminal_num);

	screen_x = x;
	screen_y = y;
	update_cursor();
	return;
}
*/


