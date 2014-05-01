/* rtc.h - Defines used in interactions with the RTC controller
 * vim:ts=4 sw=4 noexpandtab
 */

#ifndef _RTC_H
#define _RTC_H

#include "types.h"
#include "lib.h"
#include "i8259.h"

/*IO Ports used for RTC/CMOS*/
#define NMI_RTC_PORT 0x70
#define RTC_RAM_PORT 0x71

/*RTC Status Registers*/
#define REG_A	0x0A
#define REG_B	0x0B
#define REG_C	0x0C
#define REG_D 0x0D

/*NMI (Non Maskable Interrupt) Instructions*/
#define DISABLE_NMI	0x80
#define ENABLE_NMI	0x7F

/*RTC frequencies*/
#define BASE_FREQ 0x10
#define MAX_FREQ_HZ 1024
#define HZ_2	0x0F
#define HZ_4	0x0E
#define HZ_8	0x0D
#define HZ_16	0x0C
#define HZ_32	0x0B
#define HZ_64	0x0A
#define HZ_128	0x09
#define HZ_256	0x08
#define HZ_512	0x07
#define HZ_1024	0x06

/* Default RTC freq is 1024 Hz */
#define RTC_FREQ (BASE_FREQ-10)

/*RTC interrupt flag
 *set flag to 1 in rtc_read
 *clear flag with interrupt handler*/
extern uint32_t rtc_intf;

/*Initializes the RTC to IRQ 8*/
void rtc_init(void);

/*RTC interrupt handler*/
void rtc_handle_interrupt(void);

/*RTC Frequency Modifier*/
void rtc_modify_freq(uint32_t freq);

/*RTC system calls*/
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);

#endif /* _RTC_H */
