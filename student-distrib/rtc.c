/* rtc.c - Functions to interact with the RTC controller
 * vim:ts=4 noexpandtab
 */

#include "rtc.h"
#include "lib.h"

void rtc_init(void)
{
	/*Disable NMI and select register B of RTC*/
	outb(DISABLE_NMI + REG_B, NMI_RTC_PORT);

	/*Extract current value from register B*/
	char regB = inb(RTC_RAM_PORT);
	regB = regB | 0x40;							//Enable bit 6 of register B to enable Periodic Interrupts (PIE)
	
	/*Set the same index. Reading from the port sets the index to register D*/
	outb(DISABLE_NMI + REG_B, NMI_RTC_PORT);	
	
	/*Set Register B with enabled PIE*/
	outb(regB, RTC_RAM_PORT);
	
	/*Re-enable NMI*/
	outb(REG_B, NMI_RTC_PORT);
	
}
