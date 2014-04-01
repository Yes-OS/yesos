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

int8_t lctrl_held = 0;
int8_t rctl_held = 0;

#define LCTL_KEY 1000
#define RCTL_KEY 1001

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
	int idx;
	int count;
	int ok;
	int8_t *b_idx;
	int8_t c;
	int8_t *buffer = (int8_t *)buf;

	if (fd != STDIN) {
		return -1;
	}

	do {
		idx = 0;
		c = 0;
		if (!CIRC_BUF_EMPTY(term_key_buf)) {
			/* count the number of characters until the newline */
			for (idx = 0, b_idx = CIRC_BUF_IDX(term_key_buf, idx);
					b_idx != term_key_buf.tail && *b_idx != '\n' && idx < nbytes;
					idx++, b_idx = CIRC_BUF_IDX(term_key_buf, idx)) {
				/* do nothing */
			}

			/* if we've not hit the end of the buffer */
			if (b_idx != term_key_buf.tail) {
				c = *b_idx;
			}
		}
	} while (c != '\n' && idx < nbytes && !CIRC_BUF_FULL(term_key_buf));

	/* we need to copy one more byte, since idx points to the last character to copy */
	idx++;

	for (count = 0; count < idx; count++) {
		CIRC_BUF_POP(term_key_buf, c, ok);
		buffer[count] = c;
	}

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
		if ((lctrl_held || rctl_held) && key == 'l') {
			clear();
			update_cursor();
			return;
		}
		if (key == LCTL_KEY) {
			lctrl_held = 1;
			return;
		}
		if (key == RCTL_KEY) {
			rctl_held = 1;
			return;
		}
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
	else {
		if (key == LCTL_KEY) {
			lctrl_held = 0;
		}
		else if (key == RCTL_KEY) {
			rctl_held = 0;
		}
	}
}

