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
int8_t rctrl_held = 0;
int8_t lshift_held = 0;
int8_t rshift_held = 0;
int8_t caps_lock = 0;

#define IS_UPCASE (caps_lock ^ (lshift_held | rshift_held))

#define LCTL_KEY 1000
#define RCTL_KEY 1001

static const int8_t key_values[2][128] = {
	{
		   0,    0,  '`',  '1',  '2',  '3',  '4',  '5',
		 '6',  '7',  '8',  '9',  '0',  'q',  'w',  'e',
		 'r',  't',  'y',  'u',  'i',  'o',  'p',  'a',
		 's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',
		 'z',  'x',  'c',  'v',  'b',  'n',  'm',  '-',
		 '=',  '[',  ']', '\\',  ';', '\'',  ',',  '.',
		 '/',  ' ',    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
	},
	{
		   0,    0,  '~',  '!',  '@',  '#',  '$',  '%',
		 '^',  '&',  '*',  '(',  ')',  'Q',  'W',  'E',
		 'R',  'T',  'Y',  'U',  'I',  'O',  'P',  'A',
		 'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',
		 'Z',  'X',  'C',  'V',  'B',  'N',  'M',  '_',
		 '+',  '{',  '}',  '|',  ':',  '"',  '<',  '>',
		 '?',  ' ',    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
	},
};

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
	int count;
	int ok;
	int8_t *b_idx;
	int8_t c;
	int8_t *buffer = (int8_t *)buf;

	if (fd != STDIN) {
		return -1;
	}

	do {
		CIRC_BUF_POP(term_key_buf, c, ok);
		if (ok) {
			if (c == '\b') {
				if (idx > 0) {
					idx--;
					putc(c);
					update_cursor();
				}
			}
			else {
				buffer[idx++] = c;
				putc(c);
				update_cursor();
			}
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
		printf("0x%x ", key);
#if 0
		if ((lctrl_held || rctrl_held) && key == 'l') {
			/* clear the screen, update the cursor, and clear the key buffer */
			clear();
			update_cursor();
			CIRC_BUF_INIT(term_key_buf);
			CIRC_BUF_PUSH(term_key_buf, KBD_KEY_NULL, ok);
			return;
		}
		switch (key) {
			case KBD_KEY_LCTRL:
				lctrl_held = 1;
				break;
			case KBD_KEY_RCTRL:
				rctrl_held = 1;
				break;
			case KBD_KEY_LSHIFT:
				lshift_held = 1;
				break;
			case KBD_KEY_RSHIFT:
				rshift_held = 1;
				break;
			case KBD_KEY_CAPS:
				caps_lock = 1;
				break;
			default:
				if (key < 128) {
					/* printable */
					key = key_values[IS_UPCASE][key];
					if (key) {
						CIRC_BUF_PUSH(term_key_buf, key, ok);
					}
				}
				break;
		}
#endif
	}
#if 0
	else {
		switch (key) {
			case KBD_KEY_LCTRL:
				lctrl_held = 0;
				break;
			case KBD_KEY_RCTRL:
				rctrl_held = 0;
				break;
			case KBD_KEY_LSHIFT:
				lshift_held = 0;
				break;
			case KBD_KEY_RSHIFT:
				rshift_held = 0;
				break;
			case KBD_KEY_CAPS:
				caps_lock = 0;
				break;
			default:
				break;
		}
	}
#endif
}

