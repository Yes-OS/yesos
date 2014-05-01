/* term.h - terminal driver and related functions
 * vim:ts=4 sw=4 noexpandtab
 */
#ifndef _TERM_H_
#define _TERM_H_

#include "types.h"
#include "queue.h"

/* define constant file descriptors */
#define	STDIN	0
#define	STDOUT	1

/* buffer size of keyboard + 1 */
#define KBD_BUF_SIZE 129

/* definitions only available to c code */
#ifndef ASM

typedef struct terminal {
	/* Defines a circular buffer for keypresses */
	DECLARE_CIRC_BUF(int8_t, term_key_buf, KBD_BUF_SIZE);

	/* handle modifier keys */
	int8_t lctrl_held;
	int8_t rctrl_held;
	int8_t lshift_held;
	int8_t rshift_held;
	int8_t lalt_held;
	int8_t ralt_held;
	int8_t caps_lock;
} term_t;

int32_t term_open(const uint8_t *filename);
int32_t term_close(int32_t fd);
int32_t term_read(int32_t fd, void *buf, int32_t nbytes);
int32_t term_write(int32_t fd, const void *buf, int32_t nbytes);
void term_handle_keypress(uint16_t key, uint8_t status);
int32_t term_init_global_ctx();

/* forward declare to break dependency loop */
struct fops;
typedef struct fops fops_t;
extern fops_t term_fops;

extern int8_t terminal_num;

#endif

#endif
