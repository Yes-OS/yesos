/* term.c - terminal driver and related functions
 * vim:ts=4 sw=4 noexpandtab
 */

#include "lib.h"
#include "vga.h"
#include "kbd.h"
#include "term.h"

/* open terminal fd, returns STDIN */
int32_t term_open(const uint8_t *filename)
{
	(void)filename;
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
	if (fd != STDIN) {
		return -1;
	}

	return kbd_read(buf, nbytes);
}

/* if fd is STDOUT, proceed as normal, otherwise fail */
int32_t term_write(int32_t fd, const void *buf, int32_t nbytes)
{
	if (fd != STDOUT) {
		return -1;
	}

	puts((int8_t *)buf);

	return nbytes;
}
