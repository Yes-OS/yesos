/* sched.c, scheduling functions
 * vim:ts=4 sw=4 noexpandtab
 */

#include "types.h"
#include "queue.h"
#include "proc.h"
#include "x86_desc.h"
#include "i8259.h"
#include "lib.h"
#include "sched.h"

/* Helper functions */
static void context_switch(registers_t* regs);

/* Local variables */
DECLARE_CIRC_BUF(uint32_t, SCHED_QUEUE_1, MAX_PROCESSES + 1);
DECLARE_CIRC_BUF(uint32_t, SCHED_QUEUE_2, MAX_PROCESSES + 1);
sched_queue_t* active_queue;
sched_queue_t* expired_queue;
sched_flags_t sched_flags;

/* Initialize the scheduler
 * Creates two empty queues and sets flags
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
 *	Add it to the currently active queue
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
 *	Remove it from the scheduler completely
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

/* Swap the active queue with the expired queue
 *	Used when the active queue is empty
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

	/* this never actually runs because context_switch() doesn't return */
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

/* Context switching function*/
void context_switch(registers_t* regs)
{
	/*set ESP/EIP of current process by means of PID*/
	pcb_t* pcb;
	uint32_t pid, ok;

	/* if no processes are running, we have nothing to switch to */
	if (!nprocs) {
		goto leave;
	}

	/* get the current process's pcb */
	pcb = get_proc_pcb();
	if (pcb) {
		/* we're switching from another process */
		if (!(pcb->state & EXIT_DEAD)) {
			/* if we're not dead, we're expired */
			push_to_expired(pcb->pid);
		}
		else {
			//printf("DEAD! [%d]\n", pcb->pid);
			pcb->state &= ~EXIT_DEAD;
		}

		/*Set ESP/EIP for exiting process*/
		pcb->context_esp = regs;
	}

next_process:
	if (CIRC_BUF_EMPTY(*active_queue)) {
		swap_queues();
	}

	/*reload tss with new process stack info*/
	CIRC_BUF_POP(*active_queue, pid, ok);

	if (!ok) {
		puts("PANIC: nothing to resume to...\n");
		if (CIRC_BUF_EMPTY(*expired_queue)) {
			puts("PANIC: nothing in the expired queue either...\n");
		}
		else {
			/* why... */
			swap_queues();
			goto next_process;
		}
		halt();
		goto leave;
	}

	pcb = get_pcb_from_pid(pid);
	if (!pcb) {
		puts("PANIC: invalid process in sched queue\n");
		goto leave;
	}

	if (pcb->state & EXIT_DEAD) {
		goto next_process;
	}

	if (!pcb->context_esp) {
		push_to_expired(pid);
		printf("WARN: No context, returning [%d][%x]\n", pcb->pid, (uint32_t)pcb);
		goto leave;
	}

	tss.esp0 = pcb->kern_stack;
	tss.ss0 = KERNEL_DS;

	regs = pcb->context_esp;
	pcb->context_esp = NULL;

	/*reload CR3*/
	set_pdbr(pcb->page_directory);

	/* TODO: move this? */
	send_eoi(PIT_IRQ_PORT);

	asm volatile (	"movl %0, %%esp\n"
			"jmp exit_syscall"
			: : "g"(regs)
			: "cc", "memory");
leave:
	send_eoi(PIT_IRQ_PORT);
}

