/* pit.h - Defines used in interaction
 * with PIT
 * vim:ts=4 sw=4 noexpandtab
 */

#ifndef _PIT_H
#define _PIT_H


/*Ports used for PIT initialization*/
#define CHAN0_PORT   0x40
#define MODE_CMD_PORT 0x43

/*0011 1000
 * Channel 0 - 00
 * Access Mode - lobyte/highbyte - 11
 * Opearting Mode - Software Triggered Strobe - 100
 * Binary mode - 0
 */
#define INIT_CMD 0x38

/* Frequency between 10 - 50 ms
 * Divider Value
 * 5D38 = 23864
 */
#define SCHED_FREQ_LO 0x38
#define SCHED_FREQ_HI 0x5D

void pit_init(void);
void pit_handle_interrupt(void);

#endif /* _PIT_H */

