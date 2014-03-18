/* rtc.h - Defines used in interactions with the RTC controller
 * vim:ts=4 sw=4 noexpandtab
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
#define REG_D   0x0D

/*NMI (Non Maskable Interrupt) Instructions*/
#define DISABLE_NMI	0x80
#define ENABLE_NMI	0x7F


/*Initializes the RTC to IRQ 8*/
void rtc_init(void);
void rtc_handle_interrupt(void);


#endif /* _RTC_H */
