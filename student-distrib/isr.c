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

/* implements the interrupt service request */
void isr_impl(registers_t regs)
{
	uint32_t cr2;
	uint32_t cr3;
	switch (regs.isrno) {

        case EXCEPTION_DIVIDE:
            printf("Interrupt occurred(0): divide_error");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
				break;
			}
			/* just halt for now */
			halt(); 
            break;

        case EXCEPTION_DEBUG:
            printf("Interrupt occurred(1): debug");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
				break;
			}
            halt(); /* just halt for now */
            break;

        case EXCEPTION_NMI:
            printf("Interrupt occurred(2): nmi");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
				break;
			}
            halt(); /* just halt for now */
            break;

        case EXCEPTION_BREAKPOINT:
            printf("Interrupt occurred(3): breakpoint");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
				break;
			}
            halt(); /* just halt for now */
            break;

        case EXCEPTION_OVERFLOW:
            printf("Interrupt occurred(4): overflow");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
				break;
			}
            halt(); /* just halt for now */
            break;

        case EXCEPTION_BOUND:
            printf("Interrupt occurred(5): bound");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
				break;
			}
            halt(); /* just halt for now */
            break;

        case EXCEPTION_INVALID_OPCODE:
            printf("Interrupt occurred(6): invalid_opcode");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
				break;
			}
            halt(); /* just halt for now */
            break;

        case EXCEPTION_DEV_NOT_AVAIL:
            printf("Interrupt occurred(7): device_not_available");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
				break;
			}
            halt(); /* just halt for now */
            break;

        case EXCEPTION_DOUBLE_FAULT:
            printf("Interrupt occurred(8): double_fault");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
				break;
			}
            halt(); /* just halt for now */
            break;

        case EXCEPTION_COPROC_SEG_OVERRUN:
            printf("Interrupt occurred(9): coprocessor_segment_overrun");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
				break;
			}
            halt(); /* just halt for now */
            break;

        case EXCEPTION_INVALID_TSS:
            printf("Interrupt occurred(10): invalid_tss");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
				break;
			}
            halt(); /* just halt for now */
            break;

        case EXCEPTION_SEG_NOT_PRES:
            printf("Interrupt occurred(11): segment_not_present");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
				break;
			}
            halt(); /* just halt for now */
            break;

        case EXCEPTION_STACK_FAULT:
            printf("Interrupt occurred(12): stack_fault");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
				break;
			}
            halt(); /* just halt for now */
            break;

        case EXCEPTION_GENERAL_PROTECTION:
            printf("Interrupt occurred(13): general_protection");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
				break;
			}
            halt(); /* just halt for now */
            break;

        case EXCEPTION_PAGE_FAULT:
			asm("movl    %%cr2, %0"
					: "=r"(cr2)
					: :"memory");
			asm("movl    %%cr3, %0"
					: "=r"(cr3)
					: :"memory");
            printf("Interrupt occurred(14): page_fault\n");
			printf("Details:\n");
			printf("    Address: 0x%x\n", cr2);
			printf("    Page was %spresent\n", regs.errno & 0x01 ? "" : "NOT ");
			printf("    Accessed with a %s\n", regs.errno & 0x02 ? "write" : "read");
			printf("    Accessed in %s mode\n", regs.errno & 0x04 ? "user" : "supervisor");
			printf("    %saused by reserve bits set to 1 in page directory\n", regs.errno & 0x08 ? "C" : "Not c");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
				break;
			}
            halt(); /* just halt for now */
            break;

        case EXCEPTION_COPROC_ERROR:
            printf("Interrupt occurred(16): coprocessor_error");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
				break;
			}
            halt(); /* just halt for now */
            break;

        case EXCEPTION_ALIGNMENT_CHECK:
            printf("Interrupt occurred(17): alignment_check");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
				break;
			}
            halt(); /* just halt for now */
            break;

        case EXCEPTION_MACHINE_CHECK:
            printf("Interrupt occurred(18): machine_check");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
				break;
			}
            halt(); /* just halt for now */
            break;

        case EXCEPTION_SIMD_COPROC_ERR:
            printf("Interrupt occurred(19): simd_coprocessor_error");
			if (USER_MEM < regs.eip && regs.eip < USER_MEM + MB_4_OFFSET) {
				sys_halt(-1);
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
      disable_irq(PIT_IRQ_PORT);
      send_eoi(PIT_IRQ_PORT);
      pit_handle_interrupt();
      enable_irq(PIT_IRQ_PORT);
      break;

		/* handle the keyboard interrupt */
		case IRQ_KBD:
			/* mask the interrupt and immediately send EOI so we can service other interrupts */
			disable_irq(KBD_IRQ_PORT);
			send_eoi(KBD_IRQ_PORT);
			kbd_handle_interrupt();
			enable_irq(KBD_IRQ_PORT);
			break;

		/* handle the RTC interrupt */
		case IRQ_RTC:
			/* mask the interrupt and immediately send EOI so we can service other interrupts */
			disable_irq(RTC_IRQ_PORT);
			send_eoi(RTC_IRQ_PORT);
			rtc_handle_interrupt();
			enable_irq(RTC_IRQ_PORT);
			break;

		default:
			printf("Error: Interrupt unknown");
			halt();
			break;

	}
}

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

void set_task_gate(uint8_t n, uint16_t gdt)
{
	/* not yet implemented, we may not even use this */
}

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
