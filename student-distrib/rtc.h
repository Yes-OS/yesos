/* rtc.h - Defines used in interactions with the RTC controller
 * vim:ts=4 sw=4 noexpandtab
 */

#ifndef _RTC_H
#define _RTC_H

#include "types.h"
#include "proc.h"
#include "i8259.h"

/****************************************
 *            Global Defines            *
 ****************************************/

/*IO Ports used for RTC/CMOS*/
#define NMI_RTC_PORT 0x70
#define RTC_RAM_PORT 0x71

/*RTC Status Registers*/
#define REG_A   0x0A
#define REG_B   0x0B
#define REG_C   0x0C
#define REG_D   0x0D

/*NMI (Non Maskable Interrupt) Instructions*/
#define DISABLE_NMI 0x80
#define ENABLE_NMI  0x7F

/*RTC frequencies*/
#define BASE_FREQ 0x10
#define MAX_FREQ_HZ 1024
#define HZ_2    1
#define HZ_4    2
#define HZ_8    3
#define HZ_16   4
#define HZ_32   5
#define HZ_64   6
#define HZ_128  7
#define HZ_256  8
#define HZ_512  9
#define HZ_1024 10

/* Default RTC freq is 1024 Hz */
#define RTC_FREQ HZ_1024

#ifndef ASM

/****************************************
 *           Global Variables           *
 ****************************************/

/* file operations table for rtc */
extern fops_t rtc_fops;


/****************************************
 *         Function Declarations        *
 ****************************************/

/* Initializes the RTC to IRQ 8 */
void rtc_init(void);

/* RTC interrupt handler */
void rtc_handle_interrupt(void);

/* RTC Frequency Modifier */
void rtc_modify_freq(uint32_t freq);

/*System calls for rtc type files*/
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

/* Write to the RTC */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

/* Open a new RTC for a file */
int32_t rtc_open(const uint8_t* filename);

/* Close a RTC */
int32_t rtc_close(int32_t fd);

#endif /* ASM */
#endif /* _RTC_H */

