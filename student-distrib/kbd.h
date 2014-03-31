/* kbd.h, ps/2 keyboard driver implementation 
 * vim:ts=4 sw=4 noexpandtab
 */
#ifndef _KBD_H_
#define _KBD_H_

#include "types.h"

#ifndef ASM

void kbd_init();
void kbd_reset();
void kbd_handle_interrupt();

int32_t kbd_read(uint8_t *buf);

#endif

#endif
