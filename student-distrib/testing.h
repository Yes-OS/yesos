/* file_sys.h - Tests all functions of the file system
 *              Main function to be called in kernel.c
 * vim:ts=4 sw=4 noexpandtab
 */
 
#ifndef _TESTING_H
#define _TESTING_H

#include "types.h"
#include "lib.h"
#include "x86_desc.h"
#include "multiboot.h"
#include "file_sys.h"

#ifndef ASM

/* ________Defines and Vars________ */



/* ________Data Structures________ */



/* ________Function prototypes________ */


int8_t _test_read(void);
int8_t _test_file_sys(void);
int8_t _test_directory(void);

int8_t test_fs_all (void);


#endif /* ASM           */
#endif /* _TESTING_H   */

