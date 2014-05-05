/* sched.c, scheduling functions
 * vim:ts=4 sw=4 noexpandtab
 */

#include "types.h"
#include "queue.h"
#include "proc.h"
#include "x86_desc.h"
#include "i8259.h"
#include "lib.h"
#include "syscall.h"
#include "sched.h"

#ifdef MODE_DEBUG
#define DEBUG(...) printf(__VA_ARGS__)
#else
#define DEBUG(...)
#endif

/* Helper functions */
static void context_switch(registers_t* regs);

/* Local variables */
DECLARE_CIRC_BUF(uint32_t, SCHED_QUEUE_1, MAX_PROCESSES + 1);
DECLARE_CIRC_BUF(uint32_t, SCHED_QUEUE_2, MAX_PROCESSES + 1);
sched_queue_t* active_queue;
sched_queue_t* expired_queue;
sched_flags_t sched_flags;

/* Initializes the scheduler:
 * Declare Queues used for scheduling:
 *  Queue_1 starts out as active queue
 *  Queue_2 starts out as expired queue
 * Sets flags initial values
 */
void init_sched(void)
{
	/* Create and set pointers to two queues */
	CIRC_BUF_INIT(SCHED_QUEUE_1);
	CIRC_BUF_INIT(SCHED_QUEUE_2);

	active_queue = (sched_queue_t*)&SCHED_QUEUE_1;
	expired_queue = (sched_queue_t*)&SCHED_QUEUE_2;

	sched_flags.isZombie = 0;
	sched_flags.relaunch = 0;
}

/* When a new process is started:
 *  Add it to the currently active queue
 * Return 0 on success, -1 on error
 */
uint8_t push_to_active(uint32_t pid)
{
	uint32_t ok;

	CIRC_BUF_PUSH(*active_queue, pid, ok);

	if(!ok) {
		return -1;
	}

	return 0;
}

/* When a special process is started:
 *  Add it to the currently expired queue
 * Return 0 on success, -1 on error
 */
uint8_t push_to_expired(uint32_t pid)
{
	uint32_t ok;

	CIRC_BUF_PUSH(*expired_queue, pid, ok);

	if(!ok) {
		return -1;
	}

	return 0;
}

/* When a process is finished:
 *  Remove it from the scheduler completely
 *  Set it's state to DEAD
 * Return 0 on success, -1 on error
 */
uint8_t remove_active_from_sched(void)
{
#if 0
	uint32_t ok, temp;

	CIRC_BUF_POP(*active_queue, temp, ok);
	/* we don't actually use the popped value */
	(void)temp;

	if(!ok) {
		return -1;
	}
#else
	get_proc_pcb()->state |= EXIT_DEAD;
#endif

	return 0;
}

/* Push process from active queue to expired queue
 * Returns 0 on success, -1 on failure
 */
uint8_t active_to_expired(void)
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

/* Returns true if the active queue is empty */
int32_t active_empty()
{
	return CIRC_BUF_EMPTY(*active_queue);
}

/* Returns true if the expired queue is empty */
int32_t expired_empty()
{
	return CIRC_BUF_EMPTY(*expired_queue);
}

/* Swap the active queue with the expired queue
 *  Used when the active queue is empty
 */
void swap_queues(void)
{
	sched_queue_t* temp = active_queue;
	active_queue = expired_queue;
	expired_queue = temp;
}

/* Manage schedule queues and context switch
 */
void scheduler(registers_t* regs)
{
	/* context switching */
	context_switch(regs);

#if reboot
	if (sched_flags.relaunch) {
		/* When top shell is exited, must reboot */
		sched_flags.relaunch = 0;
		sys_exec((uint8_t*)"shell"); /* XXX: CAN'T DO DIS! */
	}
#else

	/* Reset flag, since not relaunching base shell */
	sched_flags.relaunch = 0;
#endif

}

/* Context switching helper function:
 *  Handles queue switching for processes*/
void context_switch(registers_t* regs)
{
	/* Set ESP/EIP of current process by means of PID*/
	pcb_t* pcb;
	uint32_t pid, ok;

	/* Get the PCB of current process */
	pcb = get_proc_pcb();

	/* If no processes are running, we have nothing to switch to */
	if (!pcb && !nprocs) {
		goto leave;
	}

	if (pcb) {
		/* We're switching from another process */
		if (!(pcb->state & EXIT_DEAD)) {
			/* If we're not dead, we're expired */
			push_to_expired(pcb->pid);
		}
		else {
			//printf("DEAD! [%d]\n", pcb->pid);
			pcb->state &= ~EXIT_DEAD;
		}

		/* Set ESP/EIP for exiting process*/
		pcb->sched_ctx = regs;
	}

	/* TODO: rewrite */
next_process:
	if (active_empty()) {
		swap_queues();
	}

	/*reload tss with new process stack info*/
	CIRC_BUF_POP(*active_queue, pid, ok);

	/* Error checking when queues empty */
	if (!ok) {
		DEBUG("PANIC: nothing to resume to...\n");
		if (expired_empty()) {
			DEBUG("PANIC: nothing in the expired queue either...\n");
		}
		else {
			/* Expired empty, swap with active...why... */
			swap_queues();
			goto next_process;
		}

		/* spawn a new shell to rescue us */
		puts("Spawning a new shell...\n");
		pid = sys_exec_internal((uint8_t*)"shell", NULL);
		if (pid > 0) {
			goto next_process;
		}
		else {
			DEBUG("PANIC: Failed to spawn a new shell\n");
		}
	}

	/* Get PCB of next Process */
	pcb = get_pcb_from_pid(pid);

	/* Error checking when PID not [fully] initialized */
	if (!pcb) {
		DEBUG("PANIC: invalid process in sched queue\n");
		goto leave;
	}

	if (pcb->state & EXIT_DEAD) {
		goto next_process;
	}

	if (!pcb->sched_ctx) {
		/* Push to be initialized later */
		push_to_expired(pid);
		DEBUG("WARN: No context, returning [%d][%x]\n", pcb->pid, (uint32_t)pcb);
		goto leave;
	}

	tss.esp0 = pcb->kern_stack;
	tss.ss0 = KERNEL_DS;

	regs = pcb->sched_ctx;
	pcb->sched_ctx = NULL;

	/*reload CR3*/
	set_pdbr(pcb->page_directory);

	/* TODO: move this? */
	send_eoi(PIT_IRQ_PORT);

	/* return to previous context */
	exit_syscall(regs);
leave:
	send_eoi(PIT_IRQ_PORT);
}

