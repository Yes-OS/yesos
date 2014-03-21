#ifndef _KBD_H_
#define _KBD_H_

#ifndef ASM

extern void kbd_init();
extern void kbd_reset();
extern void kbd_handle_interrupt();

#endif

#endif
