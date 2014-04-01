/* term.c - terminal driver and related functions
 * vim:ts=4 sw=4 noexpandtab
 */

#include "lib.h"
#include "vga.h"
#include "kbd.h"
#include "queue.h"
#include "term.h"

/* Defines a circular buffer for keypresses */
#define KBD_BUF_SIZE 129
DECLARE_CIRC_BUF(int8_t, term_key_buf, KBD_BUF_SIZE);


/* open terminal fd, returns STDIN */
int32_t term_open(const uint8_t *filename)
{
	(void)filename;
	/* initialize the keyboard buffer */
	CIRC_BUF_INIT(term_key_buf);
	return STDIN;
}

/* closes terminal fd, fails always */
int32_t term_close(int32_t *fd)
{
	(void)fd;
	return -1;
}

/* if fd is STDIN, proceed as normal, otherwise fail */
int32_t term_read(int32_t fd, void *buf, int32_t nbytes)
{
	int idx = 0;
	int ok;
	int8_t c;

	if (fd != STDIN) {
		return -1;
	}

	do {
		CIRC_BUF_POP(term_key_buf, c, ok);
		if (ok) {
			((int8_t *)buf)[idx++] = c;
		}
	} while (c != '\n' && idx < nbytes);

	return idx;
}

/* if fd is STDOUT, proceed as normal, otherwise fail */
int32_t term_write(int32_t fd, const void *buf, int32_t nbytes)
{
	int idx;

	if (fd != STDOUT) {
		return -1;
	}

	for (idx = 0; idx < nbytes; idx++) {
		putc(((int8_t *)buf)[idx]);
	}

	update_cursor();

	return idx;
}

void term_handle_keypress(uint16_t key, uint8_t status)
{
	int ok;
	/* just echo the key value for now, we'll handle things specifically later */
	if (status) {
		if (key == '\b') {
			CIRC_BUF_POP_TAIL(term_key_buf, key, ok);
			if (ok && key == '\n') {
				CIRC_BUF_PUSH(term_key_buf, key, ok);
			}
			else if (ok) {
				putc((int8_t)'\b');
				update_cursor();
			}
			return;
		}
		CIRC_BUF_PUSH(term_key_buf, key, ok);
		if (ok) {
			putc((int8_t)key);
			update_cursor();
		}
	}
}

#if 0
int32_t term_buf_has_newline()
{
	int32_t idx = 0;

	uint16_t *ptr = term_key_buf.head;

}
#endif
