/* kbd.h, ps/2 keyboard driver implementation 
 * vim:ts=4 sw=4 noexpandtab
 */
#ifndef _KBD_H_
#define _KBD_H_

#include "types.h"

#define KBD_KEY_LCTRL	0x14
#define KBD_KEY_RCTRL	0x94
#define KBD_KEY_LALT	0x11
#define KBD_KEY_RALT	0x91
#define KBD_KEY_LSUPER	0x9F
#define KBD_KEY_RSUPER	0xA7
#define KBD_KEY_LSHIFT	0x12
#define KBD_KEY_RSHIFT	0x59

#define KBD_KEY_NULL	0x00

#ifndef ASM

void kbd_init();
void kbd_reset();
void kbd_handle_interrupt();

#endif

#endif
