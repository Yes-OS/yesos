/* testing.c -  Functions to initialize and test
 *              the various systems.
 *              Generally considered a sandbox of tests
 * vim:ts=4 sw=4 noexpandtab
 */

#include "testing.h"
#include "file_sys.h"


/* Read tests
 * Three functions
 */
int8_t _test_read(void){

	/*__Read dentry by name test__*/
	puts("Testing read_dentry_by_name\n");

	int32_t retval = 0;

	/*insert file name to test for here*/
	int8_t* test_fname = "grep";
	dentry_t test_dentry;
	/*inset index number to test for here*/
	int32_t inode_test = 12;
	dentry_t test_dentry2;

	/*Test READ_DATA*/
	int32_t inode = 13;
	uint32_t offset = 1;
	uint32_t length = 4000;
	uint8_t buf[length];

	puts("Testing read_dentry_by_name\n");
	retval = read_dentry_by_name ((uint8_t*)test_fname, &test_dentry);
	if(retval == -1){
		puts("read_dentry_by_name failed. Exiting Test.\n");
		return -1;
	}

	puts("test_dentry values:\n");

	printf("Filename: %s\n",test_dentry.file_name);
	printf("Should be 2: %u\n",test_dentry.file_type);
	printf("inode_num: %u\n",test_dentry.inode);

	puts("Done with read_dentry_by_name testing\n");

	/*__Read dentry by index test__*/
	puts("Testing read_dentry_by_index\n");

	retval = read_dentry_by_index(inode_test, &test_dentry2);
	if(retval == -1){
		puts("read_dentry_by_index failed. Exiting Test.\n");
		return -1;
	}

	puts("test_dentry2 values:\n");

	printf("Filename: %s\n",test_dentry2.file_name);
	printf("Should be 2: %u\n",test_dentry2.file_type);
	printf("inode_num: %u\n",test_dentry2.inode);

	puts("Done with read_dentry_by_index testing\n");

	/*__Test read data__*/
	puts("Testing read_data\n");

	retval = read_data(inode, offset, buf, length);

	printf("READ RETURN: %d \n", retval);
	printf("\n%s\n", buf);

	if(retval == -1){
		puts("read_data failed. Exiting Test.\n");
		return -1;
	}

	return 0;

}

/* File system tests
 * read, write, open, close
 */
#if 0 /* not compliant with current version of OS */
int8_t _test_file_sys(void){
	/* reset fs pointers */
	fs_init();

	/* Hold error count for this test section */
	int8_t file_sys_count;
	file_sys_count = 0;

	/* test file open/close/write */
	file_sys_count += fs_open();
	file_sys_count += fs_close();
	file_sys_count += fs_write(0, 0, 0);
	if (file_sys_count == -1) file_sys_count = 0;
	if (file_sys_count != 0) return file_sys_count; //return if failure this far

	/* test file sys read */
	puts("HEY: fs read test not implemented yet\n");

	return file_sys_count;
}
#endif

/* Directory tests
 * read, write, open, close
 */
int8_t _test_directory(void){

	return 0;
}

/*  Perform all file system tests
 *
 *  Return 0 when all tests return no error
 *  Return -1 else
 */
int8_t test_fs_all(void){
	int8_t error_count;
	error_count = 0;

	// puts("Read tests:...\n");
	// error_count += _test_read();
	// puts("Read tests done.\n");

#if 0 /* not used right now */
	puts("File system tests:...\n");
	error_count += _test_file_sys();
	puts("File system tests done.\n");
#endif


	puts("Directory tests:...\n");
	error_count += _test_directory();
	puts("Directory tests done.\n");

	return error_count;
}

int8_t test_EIP(void)
{
	uint32_t temp_EIP;
	int32_t retval;

	/*insert file name to test for here*/
	int8_t* test_fname	= "ls";
	dentry_t test_dentry;

	retval = read_dentry_by_name ((uint8_t*)test_fname, &test_dentry);

	retval = file_loader(&test_dentry, &temp_EIP);
	if(retval == -1) {
		puts("file_loader FAIL\n");
		return 0;
	}

	printf("EIP: %u\n", temp_EIP);

	return 0;

}

/*Tests rtc_open function*/
void rtc_open_test(void)
{
	uint8_t* rtc_test = 0;
	rtc_open(rtc_test);
}

/*Tests rtc_read and rtc_write functions*/
void rtc_rw_test(void)
{
	uint32_t i = 2;
	uint32_t j = 0;
	int32_t fd_test = 0;
	int32_t nbytes_test = 4;

	while(i != 2048){
		rtc_read(fd_test, (void*)i, nbytes_test);
		puts("RTC int occurred ");
		if(j == i){
			rtc_write(fd_test, (void*)i, nbytes_test);
			i *= 2;
			if(i == 2048){
				puts("MAX FREQUENCY REACHED. STOPPING TEST");
			}
		}
		j++;
	}
}

/* void _test_array_typedef(void){

	test_t foo1;
	test_t foo2[TEST_SIZE];

	int i, j;

	//navigate array struct
	for (i = 0; i < ARRAYSIZE(foo1.element); i++){
		foo1.element[i] = i;
		printf("%d", foo1.element[i]);
	} puts("Done(1) \n");

 	//navigate array of array structs
	for (j = 0; j < ARRAYSIZE(foo2); j++){
		for (i = 0; i < ARRAYSIZE(foo1.element); i++){
			foo2[j].element[i] = i;
			printf("%d", foo2[j].element[i]);
		}
	} puts("Done(2) \n");
} */

/*  Sandbox tests
 *
 *	Used to test misc functionality
 */
/* int8_t test_misc(void){

	int8_t error_count;
	error_count = 0;

	puts("Typedef of array test:...\n");
	error_count += _test_array_typedef();
	puts("Typedef of array test done.\n");

	return error_count;
} */
