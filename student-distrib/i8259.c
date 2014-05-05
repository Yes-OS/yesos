/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 sw=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* 
 * Interrupt masks to determine which interrupts
 * are enabled and disabled 
 */
 
/* IRQs 0-7 MASK */
uint8_t master_mask; 
/* IRQs 8-15 MASK */
uint8_t slave_mask; 

/* 
 * Initialize the i8259 Programmable Interrupt Controller. 
 * initialzies both Master and Slave PIC using the correct Control-Words.
 * Writes mask for both PICs, leaving IRQ2 open for the Slaves input to the Master.
 *
 * Inputs: none
 * Outputs: none.
 *
 */
void i8259_init(void)
{
	/*Mask out all interrupts on the pic minus slave pic*/
	master_mask = 0xfb;
	slave_mask = 0xff;

	/*Write ICW1 to master and slave*/
	outb(ICW1,MASTER_8259_PORT);
	outb(ICW1,SLAVE_8259_PORT);

	/*Write ICW2 to master and slave*/
	outb(ICW2_MASTER, MASTER_8259_PORT + 1);
	outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);

	/*If in Cascade Mode, call ICW3*/
	if((ICW1 & 0x02) == 0) {
		/*Write ICW3 to master and slave*/
		outb(ICW3_MASTER, MASTER_8259_PORT + 1);
		outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);
	}

	if((ICW1 & 0x01) != 0) {
		/*Write ICW4 to master and slave*/
		outb(ICW4, MASTER_8259_PORT + 1);
		outb(ICW4, SLAVE_8259_PORT + 1);
	}

	/*Write masking values to master and slave*/
	/*PORT incremented by 1 because OCW1 is to be*/
	outb(master_mask, MASTER_8259_PORT + 1);

	/*accepted in the next port, otherwise system crashes*/
	outb(slave_mask, SLAVE_8259_PORT + 1);
}

/*
 * Enable (unmask) the specified IRQ port number.
 * Called during initialization of specific hardware devices.
 *
 * Inputs: irq_num - number of the IRQ to enable
 * Outputs: none
 *
 */
void enable_irq(uint32_t irq_num)
{
	if(irq_num < 8) {
		master_mask = master_mask & ~(1 << irq_num);
		/*PORT incremented by 1 because OCW1 is to be accepted in the next port*/
		outb(master_mask, MASTER_8259_PORT + 1);
	}
	else if(irq_num < 16) {
		slave_mask = slave_mask & ~(1 << (irq_num - 8));
		/*PORT incremented by 1 because OCW1 is to be accepted in the next port*/
		outb(slave_mask, SLAVE_8259_PORT + 1);
	}
}

/*
 * Disable (mask) the specified IRQ port number.
 * Not necessarily called for our functionality, as we never remove
 * a device during the system's runtime.
 * Used potentially for debugging purposes
 * 
 * Inputs: irq_num - number of the IRQ to disable
 * Outputs: none
 *
 */
void
disable_irq(uint32_t irq_num)
{
	if(irq_num < 8) {
		master_mask = master_mask | (1 << irq_num);
		/*PORT incremented by 1 because OCW1 is to be accepted in the next port*/
		outb(master_mask, MASTER_8259_PORT + 1);
	}
	else if(irq_num < 16) {
		slave_mask = slave_mask | (1 << (irq_num - 8));
		/*PORT incremented by 1 because OCW1 is to be accepted in the next port*/
		outb(slave_mask, SLAVE_8259_PORT + 1);
	}
}

/*
 * Send end-of-interrupt signal for the specified IRQ 
 * Called inside each devices interrupt handler.
 *
 * Inputs: irq_num - IRQ number of the device that sent the interrupt
 * Outputs: none
 *
 */
void send_eoi(uint32_t irq_num)
{
	if (irq_num < 8) {
		outb(EOI | irq_num, MASTER_8259_PORT);
	}
	else {
		outb(EOI | (irq_num - 8), SLAVE_8259_PORT);
		outb(EOI | 2, MASTER_8259_PORT);
	}
}


