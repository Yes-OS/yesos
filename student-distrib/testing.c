/* testing.c -  Functions to initialize and test
 *              the file system.
 * 
 * vim:ts=4 sw=4 noexpandtab
 */

#include "testing.h"
#include "file_sys.h"


/* Read tests 
 * Three functions
 */
int8_t _test_read(void)
{

	/*__Read dentry by name test__*/
	printf("Testing read_dentry_by_name\n");
	
	int32_t retval;
	int8_t* test_fname	= "frame1.txt"; 
	dentry_t* test_dentry;
	
	retval = read_dentry_by_name ((uint8_t*)test_fname, test_dentry);
	if(retval == -1){
		printf("read_dentry_by_name failed. Exiting Test.\n");
		return -1;
	}
	
	printf("test_dentry values:\n");
	
	printf("Filename: %s\n",test_dentry->file_name);
	printf("Should be 2: %u\n",test_dentry->file_type);
	printf("inode_num: %u\n",test_dentry->inode_num);
		
	printf("Done with read_dentry_by_name testing\n");
	
	
	/*__Read dentry by index test__*/
	printf("Testing read_dentry_by_index\n");
	
	int32_t inode_test = test_dentry->inode_num;
	dentry_t* test_dentry2;
	
	retval = read_dentry_by_index(inode_test, test_dentry2);
	if(retval == -1){
		printf("read_dentry_by_index failed. Exiting Test.\n");
		return -1;
	}
	
	printf("test_dentry2 values:\n");
	
	printf("Filename: %s\n",test_dentry2->file_name);
	printf("Should be 2: %u\n",test_dentry2->file_type);
	printf("inode_num: %u\n",test_dentry2->inode_num);
		
	printf("Done with read_dentry_by_index testing\n");
	

	return 0;

}


/* File system tests 
 * read, write, open, close
 */
int8_t _test_file_sys(void)
{
	/* reset fs pointers */
	fs_init();

	/* Hold error count for this test section */
	int8_t file_sys_count;
	file_sys_count = 0;
	
	/* test file open/close/write */
	file_sys_count += fs_open(); 			//ret 0 on success
	file_sys_count += fs_close(); 			//ret 0 on success
	file_sys_count += fs_write(0, 0, 0); 	//ret -1 on 'success'
	if (file_sys_count == -1) file_sys_count = 0;
	if (file_sys_count != 0) return file_sys_count; //return if failure this far
	
	/* test file sys read */
	printf("HEY: fs read test not implemented yet\n");
	
	return file_sys_count;
}


/* Directory tests 
 * read, write, open, close
 */
int8_t _test_directory(void)
{
	/* reset fs pointers */
	fs_init();

	/* Hold error count for this test section */
	int8_t direc_count;
	direc_count = 0;
	
	/* test directory open/close/write */
	direc_count += dir_open(); 			//ret 0 on success
	direc_count += dir_close(); 		//ret 0 on success
	direc_count += dir_write(0, 0, 0); 	//ret -1 on 'success'
	if (direc_count == -1) direc_count = 0;
	if (direc_count != 0) return direc_count; //return if failure this far
	
	/* test directory read */
	printf("HEY: Direc read test not implemented yet\n");
	
	return direc_count;
}


/*  Perform all tests
 *
 *  Return 0 when all tests return no error
 *  Return -1 else
 */
int8_t test_fs_all (void)
{
	int8_t error_count;
	error_count = 0;
	
	// printf("Read tests:...\n");
	// error_count += _test_read();
	// printf("Read tests done.\n");
	
	
	printf("File system tests:...\n");	
	error_count += _test_file_sys();
	printf("File system tests done.\n");
	
	
	printf("Directory tests:...\n");
	error_count += _test_directory();
	printf("Directory tests done.\n");

	return error_count;
}
