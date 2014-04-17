/* term.c - terminal driver and related functions
 * vim:ts=4 sw=4 noexpandtab
 */

#include "lib.h"
#include "vga.h"
#include "kbd.h"
#include "queue.h"
#include "term.h"
#include "proc.h"

/* File operations jump table */
fops_t term_fops = {
	.read  = &term_read,
	.write = &term_write,
	.open  = &term_open,
	.close = &term_close,
};


/* Defines a circular buffer for keypresses */
#define KBD_BUF_SIZE 129
DECLARE_CIRC_BUF(int8_t, term_key_buf, KBD_BUF_SIZE);

/* handle modifier keys */
int8_t lctrl_held = 0;
int8_t rctrl_held = 0;
int8_t lshift_held = 0;
int8_t rshift_held = 0;
int8_t caps_lock = 0;

uint32_t chars_since_enter = 0;

/* Translate virtual keys to printable values */
static const int8_t key_values[2][128] = {
	{
		/* lowercase codes */
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
		'\n', '\t', '\b',    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
	},
	{
		/* uppercase codes */
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
		'\n', '\t', '\b',    0,    0,    0,    0,    0,
		   0,    0,    0,    0,    0,    0,    0,    0,
	},
};

/* open terminal fd, returns STDIN */
int32_t term_open(const uint8_t *filename)
{
	(void)filename;

	/* initialize the keyboard buffer */
	CIRC_BUF_INIT(term_key_buf);

	/* clear the status flags */
	lctrl_held = 0;
	rctrl_held = 0;
	lshift_held = 0;
	rshift_held = 0;
	caps_lock = 0;

	chars_since_enter = 0;

	return STDIN;
}

/* closes terminal fd, fails always */
int32_t term_close(int32_t fd)
{
	(void)fd;
	return -1;
}

/* if fd is STDIN, proceed as normal, otherwise fail */
int32_t term_read(int32_t fd, void *buf, int32_t nbytes)
{
	int idx = 0;
	int ok;
	int8_t c = 0;
	int8_t *buffer = (int8_t *)buf;

	if (fd != STDIN) {
		return -1;
	}

	do {
		/* dequeue a character and test for backspace and screen clear */
		CIRC_BUF_POP(term_key_buf, c, ok);
		if (ok) {
			if (c == '\b') {
				if (idx > 0) {
					idx--;
				}
			}
			else if (c == KBD_KEY_NULL) {
				idx = 0;
			}
			else {
				/* push the character into the buffer */
				buffer[idx++] = c;
			}
		}
	} while (c != '\n' && idx < nbytes);

	/* prevent backspace past last feed */
	chars_since_enter = 0;

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
		if ((lctrl_held || rctrl_held) && key == KBD_KEY_L) {
			/* clear the screen, update the cursor, and clear the key buffer */
			clear();
			update_cursor();
			chars_since_enter = 0;
			CIRC_BUF_INIT(term_key_buf);
			CIRC_BUF_PUSH(term_key_buf, KBD_KEY_NULL, ok);
			if (!ok) {
				printf("ERR: failed when clearing screen\n");
			}
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
				caps_lock ^= 1;
				break;
			default:
				if (key < 128) {
					/* printable */
					/* toggle alphanumeric when capslock is on */
					if (caps_lock && (key >= KBD_KEY_Q && key <= KBD_KEY_M)) {
						key = key_values[caps_lock ^ (lshift_held | rshift_held)][key];
					}
					else {
						/* handle symbols as normal */
						key = key_values[lshift_held | rshift_held][key];
					}
					if (key) {
						/* queue the key in the buffer */
						CIRC_BUF_PUSH(term_key_buf, key, ok);

						/* if it's a backspace and there are characters since the last enter, go back */
						if (key == '\b') {
							if (chars_since_enter > 0) {
								chars_since_enter--;
								putc((int8_t)key);
							}
						}
						else if (key == '\n') {
							chars_since_enter = 0;
							putc((int8_t)key);
						}
						else {
							chars_since_enter++;
							putc((int8_t)key);
						}
						update_cursor();
					}
				}
				break;
		}
	}
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
			default:
				break;
		}
	}
}

