/* kbd.h, ps/2 keyboard driver implementation
 * vim:ts=4 sw=4 noexpandtab
 */
#ifndef _KBD_H_
#define _KBD_H_

#include "types.h"

/****************************************
 *            Global Defines            *
 ****************************************/

/* define virtual keys */
#define KBD_KEY_NULL                0x00

#define KBD_KEY_ESC                 0x01
#define KBD_KEY_BTICK               0x02

#define KBD_KEY_1                   0x03
#define KBD_KEY_2                   0x04
#define KBD_KEY_3                   0x05
#define KBD_KEY_4                   0x06
#define KBD_KEY_5                   0x07
#define KBD_KEY_6                   0x08
#define KBD_KEY_7                   0x09
#define KBD_KEY_8                   0x0A
#define KBD_KEY_9                   0x0B
#define KBD_KEY_0                   0x0C

#define KBD_KEY_Q                   0x0D
#define KBD_KEY_W                   0x0E
#define KBD_KEY_E                   0x0F
#define KBD_KEY_R                   0x10
#define KBD_KEY_T                   0x11
#define KBD_KEY_Y                   0x12
#define KBD_KEY_U                   0x13
#define KBD_KEY_I                   0x14
#define KBD_KEY_O                   0x15
#define KBD_KEY_P                   0x16
#define KBD_KEY_A                   0x17
#define KBD_KEY_S                   0x18
#define KBD_KEY_D                   0x19
#define KBD_KEY_F                   0x1A
#define KBD_KEY_G                   0x1B
#define KBD_KEY_H                   0x1C
#define KBD_KEY_J                   0x1D
#define KBD_KEY_K                   0x1E
#define KBD_KEY_L                   0x1F
#define KBD_KEY_Z                   0x20
#define KBD_KEY_X                   0x21
#define KBD_KEY_C                   0x22
#define KBD_KEY_V                   0x23
#define KBD_KEY_B                   0x24
#define KBD_KEY_N                   0x25
#define KBD_KEY_M                   0x26

#define KBD_KEY_MINUS               0x27
#define KBD_KEY_EQUALS              0x28
#define KBD_KEY_LBRACKET            0x29
#define KBD_KEY_RBRACKET            0x2A
#define KBD_KEY_BSLASH              0x2B
#define KBD_KEY_SEMICOLON           0x2C
#define KBD_KEY_SQUOTE              0x2D
#define KBD_KEY_COMMA               0x2E
#define KBD_KEY_PERIOD              0x2F
#define KBD_KEY_FSLASH              0x30
#define KBD_KEY_SPACE               0x31

#define KBD_KEY_RETURN              0x70
#define KBD_KEY_TAB                 0x71
#define KBD_KEY_BKSP                0x72

/* define special and modifier keys */
#define KBD_KEY_CAPS                0x100
#define KBD_KEY_LSHIFT              0x101
#define KBD_KEY_RSHIFT              0x102
#define KBD_KEY_LCTRL               0x103
#define KBD_KEY_RCTRL               0x104
#define KBD_KEY_LALT                0x105
#define KBD_KEY_RALT                0x106
#define KBD_KEY_LSUPER              0x107
#define KBD_KEY_RSUPER              0x108

#ifndef ASM


/****************************************
 *           Global Variables           *
 ****************************************/

extern volatile int32_t kbd_initialized;


/****************************************
 *         Function Declarations        *
 ****************************************/

void kbd_init();
void kbd_reset();
void kbd_handle_interrupt();

#endif

#endif
