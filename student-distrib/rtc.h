/* rtc.h - Defines used in interactions with the RTC controller
 * vim:ts=4 noexpandtab
 */
 
#ifndef _RTC_H
#define _RTC_H

#include "types.h"
#include "lib.h"


/*IO Ports used for RTC/CMOS*/
#define NMI_RTC_PORT 0x70
#define RTC_RAM_PORT 0x71

/*RTC Status Registers*/
#define REG_A	0x0A
#define REG_B	0x0B
#define REG_C	0x0C

/*NMI (Non Maskable Interrupt) Instruction*/
#define DISABLE_NMI	0x80


/*Initializes the RTC to IRQ 8*/
void rtc_init(void);


#endif /* _RTC_H */