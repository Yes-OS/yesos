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
#include "rtc.h"

#ifndef ASM

/* ________Defines and Vars________ */
#define TEST_SIZE 10

/* ARRAYSIZE(arr) is like an overloaded version of sizeof() that accounts for pointers being passed in */
#define IS_INDEXABLE(arg) (sizeof(arg[0]))
#define IS_ARRAY(arg) (IS_INDEXABLE(arg) && (((void *) &arg) == ((void *) arg)))
#define ARRAYSIZE(arr) (sizeof(arr) / (IS_ARRAY(arr) ? sizeof(arr[0]) : 0))


/* ________Data Structures________ */

/* typedef struct test {
	int8_t element[TEST_SIZE];
} test_t; */


/* ________Function prototypes________ */


int8_t _test_read(void);
int8_t _test_file_sys(void);
int8_t _test_directory(void);

int8_t test_fs_all (void);
int8_t test_EIP(void);

/*RTC Test Functions*/
void rtc_rw_test(void);
void rtc_open_test(void);

/* void _test_array_typedef(void); */


#endif /* ASM          */
#endif /* _TESTING_H   */

