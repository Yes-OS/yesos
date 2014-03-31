/* file_sys.c - Functions to initialize and interact
 *              with the file system.
 * 
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
 *				dentry	- pointer to directory entry of file to set name.
 *
 *  Fill 'dentry_t' block passed in with:
 *		-file name, file type, and inode number
 *
 *	Return 0 on success, -1 on failure.
 *	
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
{
	//Check for non-existant file or invalid index
	if(true)
	{
		return -1;
	}

//	dentry->file_name = fname;

// use memcpy

	return 0;
}

/*	Read Directory Entry by Name
 *	Parameters:	fname 	- the name of the file.
 *				dentry	- pointer to directory entry of file to set name.
 *
 *  Fill 'dentry_t' block passed in with:
 *		-file name, file type, and inode number
 *
 *	Return 0 on success, -1 on failure.
 *	
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{

//	!!	Check that index is less than number of indices (boot block)
//	if(index < <global boot block>.numnodes)
//		dentry->inode_num = index;

	//Check for non-existant file or invalid index
	if(true)
	{
		return -1;
	}


// use memcpy

	return 0;
}

/*	Read Data
 *	Parameters:	inode	- the index node of the data.
 *				offset	- how many bytes to begin reading at?
 *				buf		- data array to read.
 *				length  - size of array.
 *
 *  Reads up to 'length' bytes from position 'offset'
 *	  in the file with 'inode' number into the given 'buf' buffer.
 *
 *	Return  # of bytes read and placed into buffer
 *			0 when end of file reached.
 *			-1 on failure.
 *
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	unsigned int num_bytes_read;
	unsigned int end_of_file; //placeholder
	
	num_bytes_read = 1234;
	end_of_file = 1;
	
	//Check that the given inode is within the valid range
	if(true)
	{
		return -1
	}

// use memcpy

	if(end_of_file)
	{
		return 0;
	}
	
	return (num_bytes_read);

}
