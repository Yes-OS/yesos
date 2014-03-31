/* file_sys.c - Functions to initialize and interact
 *              with the file system.
 * vim:ts=4 sw=4 noexpandtab
 */

#include "file_sys.h"

/* Sets up the head pointer for the file system
 * Sets up relevant structures and variables
 *
 * Inputs:	pointer to fs head
 * Outputs:	none
 */
void fs_init(uint32_t * beg)
{
	fs_head = beg;
}

/* 
 *
 */
uint32_t fs_read(uint8_t* buf, int count)
{
	return 0;
}

/* 
 *
 */
uint32_t fs_write(/**/)
{
	return -1;
}

/* 
 *
 */
uint32_t fs_open(/**/)
{
	return 0;
}

/* 
 *
 */
uint32_t fs_close(/**/)
{
	return 0;
}


/*	Read Directory Entry by Name
 *	Parameters:	fname 	- the name of the file.
 *			dentry	- pointer to directory entry of file to set name.
 *	Return -1 on failure.
 *	
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
{
	(*dentry).file_name = fname;
	return 0;
}

/*	Read Directory Entry by Index
 *	Parameters:	index	- the index of the file.
 *			dentry	- pointer to directory entry of file to set index.
 *	Return -1 on failure.
 *	
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
	//	!!	Check that index is less than number of indices (boot block)
//	if(index < <global boot block>.numnodes)
		(*dentry).inode_num = index;
	else return -1;
	return 0;
}

/*	Read Data
 *	Parameters:	inode	- the index node of the data.
 *			offset	- how many bytes to begin reading at?
 *			buf	- data array to read.
 *			length - size of array.
 *	Return -1 on failure.
 *
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	return -1;
}
