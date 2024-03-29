/* isr.c, implement interrupt request handling and related functions
 * vim:ts=4 sw=4 noexpandtab
 */

#include "lib.h"
#include "x86_desc.h"
#include "i8259.h"
#include "isr_stub.h"
#include "types.h"
#include "kbd.h"
#include "rtc.h"
#include "pit.h"
#include "paging.h"
#include "syscall.h"
#include "proc.h"
#include "isr.h"

/* 
 * Implements the interrupt service request. 
 * Large switch statement based on the isr number, specifying what
 * type of interrupt or exception happened, and is handled accordingly
 * depending ont he type.
 *
 * Inputs: regs - registers that are pushed onto the stack prior to 
 *                interrupt handling
 * Outputs: none
 *
 */
void isr_impl(registers_t regs)
{
	uint32_t cr2;
	uint32_t cr3;
	switch (regs.isrno) {

		case EXCEPTION_DIVIDE:
			puts("Interrupt occurred(0): divide_error\n");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			/* just halt for now */
			halt();
			break;

		case EXCEPTION_DEBUG:
			puts("Interrupt occurred(1): debug\n");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			halt(); /* just halt for now */
			break;

		case EXCEPTION_NMI:
			puts("Interrupt occurred(2): nmi\n");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			halt(); /* just halt for now */
			break;

		case EXCEPTION_BREAKPOINT:
			puts("Interrupt occurred(3): breakpoint\n");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			halt(); /* just halt for now */
			break;

		case EXCEPTION_OVERFLOW:
			puts("Interrupt occurred(4): overflow\n");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			halt(); /* just halt for now */
			break;

		case EXCEPTION_BOUND:
			puts("Interrupt occurred(5): bound\n");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			halt(); /* just halt for now */
			break;

		case EXCEPTION_INVALID_OPCODE:
			puts("Interrupt occurred(6): invalid_opcode\n");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			halt(); /* just halt for now */
			break;

		case EXCEPTION_DEV_NOT_AVAIL:
			puts("Interrupt occurred(7): device_not_available\n");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			halt(); /* just halt for now */
			break;

		case EXCEPTION_DOUBLE_FAULT:
			puts("Interrupt occurred(8): double_fault\n");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			halt(); /* just halt for now */
			break;

		case EXCEPTION_COPROC_SEG_OVERRUN:
			puts("Interrupt occurred(9): coprocessor_segment_overrun\n");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			halt(); /* just halt for now */
			break;

		case EXCEPTION_INVALID_TSS:
			puts("Interrupt occurred(10): invalid_tss\n");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			halt(); /* just halt for now */
			break;

		case EXCEPTION_SEG_NOT_PRES:
			puts("Interrupt occurred(11): segment_not_present\n");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			halt(); /* just halt for now */
			break;

		case EXCEPTION_STACK_FAULT:
			puts("Interrupt occurred(12): stack_fault\n");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			halt(); /* just halt for now */
			break;

		case EXCEPTION_GENERAL_PROTECTION:
			puts("Interrupt occurred(13): general_protection\n");
			if (regs.errno > 0) {
				printf("    Segment selector: %d\n", regs.errno);
			}
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			halt(); /* just halt for now */
			break;

		case EXCEPTION_PAGE_FAULT:
			/* get CR2 */
			asm("movl    %%cr2, %0"
					: "=r"(cr2)
					: :"memory");
			/* get CR3 */
			asm("movl    %%cr3, %0"
					: "=r"(cr3)
					: :"memory");
			puts("Interrupt occurred(14): page_fault\n");
			puts("Details:\n");
			printf("    Address: 0x%x\n", cr2);
			printf("    Page was %spresent\n", regs.errno & 0x01 ? "" : "NOT ");
			printf("    Accessed with a %s\n", regs.errno & 0x02 ? "write" : "read");
			printf("    Accessed in %s mode\n", regs.errno & 0x04 ? "user" : "supervisor");
			printf("    %saused by reserve bits set to 1 in page directory\n", regs.errno & 0x08 ? "C" : "Not c");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			halt(); /* just halt for now */
			break;

		case EXCEPTION_COPROC_ERROR:
			puts("Interrupt occurred(16): coprocessor_error\n");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			halt(); /* just halt for now */
			break;

		case EXCEPTION_ALIGNMENT_CHECK:
			puts("Interrupt occurred(17): alignment_check\n");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			halt(); /* just halt for now */
			break;

		case EXCEPTION_MACHINE_CHECK:
			puts("Interrupt occurred(18): machine_check\n");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			halt(); /* just halt for now */
			break;

		case EXCEPTION_SIMD_COPROC_ERR:
			puts("Interrupt occurred(19): simd_coprocessor_error\n");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + OFFSET_4MB) {
				sys_halt_internal(get_proc_pcb()->pid, 256);
				break;
			}
			halt(); /* just halt for now */
			break;

			/*
			 * These are IRQs for which we have no specific handling code, here for debugging purposes.
			 * NOTE: These generally shouldn't be reached, since their respective lines will be masked
			 *       the PIC.
			 */
		case IRQ2:
		case IRQ3:
		case IRQ4:
		case IRQ5:
		case IRQ6:
		case IRQ7:
		case IRQ9:
		case IRQ10:
		case IRQ11:
		case IRQ12:
		case IRQ13:
		case IRQ14:
		case IRQ15:
			printf("Unhandled IRQ(%d)\n", regs.isrno - IRQ_START);
			send_eoi(regs.isrno - IRQ_START);
			break;

			/*handle PIT interrupt*/
		case IRQ_PIT:
			/* mask the interrupt and immediately send EOI so we can service other interrupts */
			pit_handle_interrupt(&regs);
			break;

			/* handle the keyboard interrupt */
		case IRQ_KBD:
			/* mask the interrupt and immediately send EOI so we can service other interrupts */
			kbd_handle_interrupt();
			send_eoi(KBD_IRQ_PORT);
			break;

			/* handle the RTC interrupt */
		case IRQ_RTC:
			/* mask the interrupt and immediately send EOI so we can service other interrupts */
			rtc_handle_interrupt();
			send_eoi(RTC_IRQ_PORT);
			break;

		default:
			puts("Error: Interrupt unknown\n");
			halt();
			break;

	}
}


/*
 * Sets an interrupt gate, the most commonly used gate type.
 * Only accesssible at the kernel level.
 *
 * Inputs: n - gate number
 *         addr - offset of the IDT entry
 * Outputs: none
 *
 */
void set_intr_gate(uint8_t n, uint32_t addr)
{
	idt_desc_t new_idt_entry;
	memset(&new_idt_entry, 0, sizeof(idt_desc_t));

	/* enables the idt entry */
	new_idt_entry.present = 1;

	/* defines 32-bit trap interrupt */
	/* touch this code and I'll end you - adam */
	new_idt_entry.size = 1;
	new_idt_entry.reserved1 = 1;
	new_idt_entry.reserved2 = 1;
	new_idt_entry.reserved3 = 0;

	/* Ring 0 level access only */
	new_idt_entry.dpl = 0;
	new_idt_entry.seg_selector = KERNEL_CS;

	/* initialize the offset */
	SET_IDT_ENTRY(new_idt_entry, addr);

	/* store IDT */
	idt[n] = new_idt_entry;
}


/*
 * Sets system gate. Only used for three exceptions.
 * Allowed to be called by user level code.
 *
 * Inputs: n - Interrupt/exception number
 *         addr - offset of the IDT entry
 * Outputs:
 *
 */
void set_system_gate(uint8_t n, uint32_t addr)
{
	idt_desc_t new_idt_entry;
	memset(&new_idt_entry, 0, sizeof(idt_desc_t));

	/* enables the idt entry */
	new_idt_entry.present = 1;

	/* defines 32-bit trap interrupt */
	/* touch this code and I'll end you - adam */
	new_idt_entry.size = 1;
	new_idt_entry.reserved1 = 1;
	new_idt_entry.reserved2 = 1;
	new_idt_entry.reserved3 = 1;

	/* Can be called by user level code */
	new_idt_entry.dpl = 3;
	new_idt_entry.seg_selector = KERNEL_CS;

	/* initialize the offset */
	SET_IDT_ENTRY(new_idt_entry, addr);

	/* store IDT */
	idt[n] = new_idt_entry;
}

/*
 * Sets system interrupt gate. Only used for one exception.
 * Allowed to be called by user level code.
 *
 * Inputs: n - Interrupt/exception number
 *         addr - offset of the IDT entry
 * Outputs:
 *
 */
void set_system_intr_gate(uint8_t n, uint32_t addr)
{
	idt_desc_t new_idt_entry;
	memset(&new_idt_entry, 0, sizeof(idt_desc_t));

	/* enables the idt entry */
	new_idt_entry.present = 1;

	/* defines 32-bit trap interrupt */
	/* touch this code and I'll end you - adam */
	new_idt_entry.size = 1;
	new_idt_entry.reserved1 = 1;
	new_idt_entry.reserved2 = 1;
	new_idt_entry.reserved3 = 0;

	/* Can be called by user level code */
	new_idt_entry.dpl = 3;
	new_idt_entry.seg_selector = KERNEL_CS;

	/* initialize the offset */
	SET_IDT_ENTRY(new_idt_entry, addr);

	/* store IDT */
	idt[n] = new_idt_entry;
}


/*
 * Sets trap gate. Not actually used for any of our exceptions/interrupts
 * Allowed to be called by user level code.
 *
 * Inputs: n - Interrupt/exception number
 *         addr - offset of the IDT entry
 * Outputs:
 *
 */
void set_trap_gate(uint8_t n, uint32_t addr)
{
	idt_desc_t new_idt_entry;
	memset(&new_idt_entry, 0, sizeof(idt_desc_t));

	/* enables the idt entry */
	new_idt_entry.present = 1;

	/* defines 32-bit trap interrupt */
	/* touch this code and I'll end you - adam */
	new_idt_entry.size = 1;
	new_idt_entry.reserved1 = 1;
	new_idt_entry.reserved2 = 1;
	new_idt_entry.reserved3 = 0;

	/* Cannot be called by user level code */
	new_idt_entry.dpl = 0;
	new_idt_entry.seg_selector = KERNEL_CS;

	/* initialize the offset */
	SET_IDT_ENTRY(new_idt_entry, addr);

	/* store IDT */
	idt[n] = new_idt_entry;
}

/*
 * Sets a task gate. Not actually used at all.
 * Used when referencing TSS descriptor in the GDT.
 *
 * Inputs: n - interrupt/exception number
 *         gdt - offset into the gdt
 * Outputs: none
 *
 */
void set_task_gate(uint8_t n, uint16_t gdt)
{
	/* not yet implemented, we may not even use this */
	(void)n; (void)gdt;
}

/*
 * Sets all the gates corresponding to all exceptions/interrupts.
 * Initialze the sys_call vector to 0x80
 * Loads the Interrupt Descriptor Table.
 *
 * Inputs: none
 * Outputs: none
 *
 */
void install_interrupts()
{
	int i;

	/* set default handler */
	for (i = 0; i < NUM_VEC; i++) {
		set_trap_gate(i, (uint32_t)&null_int);
	}

	/* initialize exceptions */
	set_intr_gate(0,  (uint32_t)&divide_error);
	set_intr_gate(1,  (uint32_t)&debug);
	set_intr_gate(2,  (uint32_t)&nmi);
	set_system_intr_gate(3,  (uint32_t)&breakpoint);
	set_system_gate(4,  (uint32_t)&overflow);
	set_system_gate(5,  (uint32_t)&bound);
	set_intr_gate(6,  (uint32_t)&invalid_opcode);
	set_intr_gate(7,  (uint32_t)&device_not_available);
	set_intr_gate(8,  (uint32_t)&double_fault);
	set_intr_gate(9,  (uint32_t)&coprocessor_segment_overrun);
	set_intr_gate(10, (uint32_t)&invalid_tss);
	set_intr_gate(11, (uint32_t)&segment_not_present);
	set_intr_gate(12, (uint32_t)&stack_fault);
	set_intr_gate(13, (uint32_t)&general_protection);
	set_intr_gate(14, (uint32_t)&page_fault);
	set_intr_gate(16, (uint32_t)&coprocessor_error);
	set_intr_gate(17, (uint32_t)&alignment_check);
	set_intr_gate(18, (uint32_t)&machine_check);
	set_intr_gate(19, (uint32_t)&simd_coprocessor_error);

	/* initialize irqs */
	set_intr_gate(32, (uint32_t)&irq0);
	set_intr_gate(33, (uint32_t)&irq1);
	set_intr_gate(34, (uint32_t)&irq2);
	set_intr_gate(35, (uint32_t)&irq3);
	set_intr_gate(36, (uint32_t)&irq4);
	set_intr_gate(37, (uint32_t)&irq5);
	set_intr_gate(38, (uint32_t)&irq6);
	set_intr_gate(39, (uint32_t)&irq7);
	set_intr_gate(40, (uint32_t)&irq8);
	set_intr_gate(41, (uint32_t)&irq9);
	set_intr_gate(42, (uint32_t)&irq10);
	set_intr_gate(43, (uint32_t)&irq11);
	set_intr_gate(44, (uint32_t)&irq12);
	set_intr_gate(45, (uint32_t)&irq13);
	set_intr_gate(46, (uint32_t)&irq14);
	set_intr_gate(47, (uint32_t)&irq15);

	/* initialize system call vector*/
	set_system_gate(0x80, (uint32_t)&enter_syscall);

	/* load IDT */
	lidt(idt_desc_ptr);
}
