/* testing.c -  Functions to initialize and test
 *              the file system.
 * 
 * vim:ts=4 sw=4 noexpandtab
 */

#include "testing.h"

uint32_t test_buf[128];

/* Read tests 
 */
uint8_t _test_read(void)
{
	return -1;
}


/* File system tests 
 */
uint8_t _test_file_sys(void)
{
	return -1;
}


/* Directory tests 
 */
uint8_t _test_directory(void)
{
	return -1;
}


/*  Perform all tests
 *
 *  Return 0 when all tests return no error
 *  Return -1 else
 */
uint8_t test_fs_all (void)
{
	uint8_t error_count;
	error_count = 0;
	
	printf("Read tests:...\n");
	error_count += _test_read();
	printf("Read tests done.\n");
	
	
	printf("File system tests:...\n");	
	error_count += _test_file_sys();
	printf("File system tests done.\n");
	
	
	printf("Directory tests:...");
	error_count += _test_directory();
	printf("Directory tests done.\n");

	return error_count;
}
