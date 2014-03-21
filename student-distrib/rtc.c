/* rtc.c - Functions to interact with the RTC controller
 * vim:ts=4 sw=4 noexpandtab
 */

#include "rtc.h"
#include "lib.h"

/* initialize the RTC */
void rtc_init(void)
{
    char regB;

	/*Disable NMI and select register B of RTC*/
	outb((DISABLE_NMI | REG_B), NMI_RTC_PORT);

	/*Extract current value from register B*/
	regB = inb(RTC_RAM_PORT);

	regB = regB | 0x40;		//Enable bit 6 of register B to enable Periodic Interrupts (PIE)
	
	/*Set the same index, because reading from the port sets the index to register D*/
	outb((DISABLE_NMI | REG_B), NMI_RTC_PORT);	
	
	/*Set Register B with enabled PIE*/
	outb(regB, RTC_RAM_PORT);

	/*Re-enable NMI*/
	char enable = inb(NMI_RTC_PORT);
	enable = enable & ENABLE_NMI;
	outb(enable, NMI_RTC_PORT);
}

/* handle the rtc interrupt */
void rtc_handle_interrupt()
{
	/* read a byte from reg c to allow interrupts to continue */
	outb(REG_C, NMI_RTC_PORT);
	inb(RTC_RAM_PORT);

	/* do the test interrupts funciton so we know things are working */
	test_interrupts();
}
