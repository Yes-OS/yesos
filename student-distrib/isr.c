/* isr.c, implement interrupt request handling and related functions
 * vim:ts=4 noexpandtab
 */

#include "isr.h"
#include "lib.h"
#include "x86_desc.h"
#include "i8259.h"
#include "isr_stub.h"
#include "types.h"
#include "kbd.h"

void isr_impl(registers_t regs)
{
	switch (regs.isrno) {

        case 0:
            printf("Interrupt occurred(0): divide_error");
            halt(); /* just halt for now */
            break;

        case 1:
            printf("Interrupt occurred(1): debug");
            halt(); /* just halt for now */
            break;

        case 2:
            printf("Interrupt occurred(2): nmi");
            halt(); /* just halt for now */
            break;

        case 3:
            printf("Interrupt occurred(3): breakpoint");
            halt(); /* just halt for now */
            break;

        case 4:
            printf("Interrupt occurred(4): overflow");
            halt(); /* just halt for now */
            break;

        case 5:
            printf("Interrupt occurred(5): bound");
            halt(); /* just halt for now */
            break;

        case 6:
            printf("Interrupt occurred(6): invalid_opcode");
            halt(); /* just halt for now */
            break;

        case 7:
            printf("Interrupt occurred(7): device_not_available");
            halt(); /* just halt for now */
            break;

        case 8:
            printf("Interrupt occurred(8): double_fault");
            halt(); /* just halt for now */
            break;

        case 9:
            printf("Interrupt occurred(9): coprocessor_segment_overrun");
            halt(); /* just halt for now */
            break;

        case 10:
            printf("Interrupt occurred(10): invalid_tss");
            halt(); /* just halt for now */
            break;

        case 11:
            printf("Interrupt occurred(11): segment_not_present");
            halt(); /* just halt for now */
            break;

        case 12:
            printf("Interrupt occurred(12): stack_fault");
            halt(); /* just halt for now */
            break;

        case 13:
            printf("Interrupt occurred(13): general_protection");
            halt(); /* just halt for now */
            break;

        case 14:
            printf("Interrupt occurred(14): page_fault");
            halt(); /* just halt for now */
            break;

        case 16:
            printf("Interrupt occurred(16): coprocessor_error");
            halt(); /* just halt for now */
            break;

        case 17:
            printf("Interrupt occurred(17): alignment_check");
            halt(); /* just halt for now */
            break;

        case 18:
            printf("Interrupt occurred(18): machine_check");
            halt(); /* just halt for now */
            break;

        case 19:
            printf("Interrupt occurred(19): simd_coprocessor_error");
            halt(); /* just halt for now */
            break;

		case 32:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 41:
		case 42:
		case 43:
		case 44:
		case 45:
		case 46:
		case 47:
			printf("Fuck the IRQ number %d\n", regs.isrno - 32);
			send_eoi(regs.isrno - 32);
			break;

		case 33:
			kbd_handle_interrupt();
			send_eoi(KBD_IRQ_PORT);
			break;

		case 40:
			test_interrupts();
			outb(0x0C, 0x70);
			inb(0x71);
			send_eoi(RTC_IRQ_PORT);
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

	/* Can be called by user level code */
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
	set_trap_gate(0,  (uint32_t)&divide_error);
	set_trap_gate(1,  (uint32_t)&debug);
	set_trap_gate(2,  (uint32_t)&nmi);
	set_trap_gate(3,  (uint32_t)&breakpoint);
	set_trap_gate(4,  (uint32_t)&overflow);
	set_trap_gate(5,  (uint32_t)&bound);
	set_trap_gate(6,  (uint32_t)&invalid_opcode);
	set_trap_gate(7,  (uint32_t)&device_not_available);
	set_trap_gate(8,  (uint32_t)&double_fault);
	set_trap_gate(9,  (uint32_t)&coprocessor_segment_overrun);
	set_trap_gate(10, (uint32_t)&invalid_tss);
	set_trap_gate(11, (uint32_t)&segment_not_present);
	set_trap_gate(12, (uint32_t)&stack_fault);
	set_trap_gate(13, (uint32_t)&general_protection);
	set_trap_gate(14, (uint32_t)&page_fault);
	set_trap_gate(16, (uint32_t)&coprocessor_error);
	set_trap_gate(17, (uint32_t)&alignment_check);
	set_trap_gate(18, (uint32_t)&machine_check);
	set_trap_gate(19, (uint32_t)&simd_coprocessor_error);

	/* initialize irqs */
	set_trap_gate(32, (uint32_t)&irq0);
	set_trap_gate(33, (uint32_t)&irq1);
	set_trap_gate(34, (uint32_t)&irq2);
	set_trap_gate(35, (uint32_t)&irq3);
	set_trap_gate(36, (uint32_t)&irq4);
	set_trap_gate(37, (uint32_t)&irq5);
	set_trap_gate(38, (uint32_t)&irq6);
	set_trap_gate(39, (uint32_t)&irq7);
	set_trap_gate(40, (uint32_t)&irq8);
	set_trap_gate(41, (uint32_t)&irq9);
	set_trap_gate(42, (uint32_t)&irq10);
	set_trap_gate(43, (uint32_t)&irq11);
	set_trap_gate(44, (uint32_t)&irq12);
	set_trap_gate(45, (uint32_t)&irq13);
	set_trap_gate(46, (uint32_t)&irq14);
	set_trap_gate(47, (uint32_t)&irq15);

	/* load IDT */
	lidt(idt_desc_ptr);
}
