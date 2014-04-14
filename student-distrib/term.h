/* term.h - terminal driver and related functions
 * vim:ts=4 sw=4 noexpandtab
 */
#ifndef _TERM_H_
#define _TERM_H_

#include "types.h"

/* define constant file descriptors */
#define	STDIN	0
#define	STDOUT	1

/* definitions only available to c code */
#ifndef ASM

int32_t term_open(const uint8_t *filename);
int32_t term_close(int32_t *fd);
int32_t term_read(int32_t fd, void *buf, int32_t nbytes);
int32_t term_write(int32_t fd, const void *buf, int32_t nbytes);
void term_handle_keypress(uint16_t key, uint8_t status);
extern void *term_fops[];

#endif

#endif
