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
 uint32_t* node_head, * data_head;
 boot_block_t * boot_block;
 
void fs_init(uint32_t * beg)
{
	fs_head = beg;
	
	boot_block = (boot_block_t *)fs_head;
	node_head = boot_block + ADDRESSES_PER_BLOCK;
	data_head = node_head + (boot_block->num_nodes)*ADDRESSES_PER_BLOCK;
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
	
	/*start from dentry index 1 because index 0 is current directory*/
	uint8_t i = 1;
	
	while(i <= boot_block->num_entries){
	
		if(!(strncmp(fname, boot_block->entries[i].file_name, FILE_NAME_SIZE))){
		
			dentry->file_name =	fname;
			dentry->file_type = boot_block->entries[i].file_type;
			dentry->inode_num = i;
		
			return 0;
		}
		
		i++;
		
	}

	return -1;
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
	if(index <= boot_block->num_nodes)
	{
	
		dentry->file_name =	boot_block->entries[index].file_name;
		dentry->file_type = boot_block->entries[index].file_type;
		dentry->inode_num = boot_block->entries[index].inode_num;
	
		return 0;
		
	}

	return -1;
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
 *  Return  # of bytes read and placed into buffer
 *			0 when end of file reached.
 *			-1 on failure.
 *
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	//unsigned int num_bytes_read;
	uint32_t bytes_read = 0;
	
	//Check that the given inode is within the valid range
	if(inode > boot_block->num_nodes)
	{
		return -1;
	}
	
	index_node_t* my_inode = node_head + inode * ADDRESSES_PER_BLOCK; 
	
	//	If the offset is beyond the block length, 0 bytes were written.
	if(my_inode->byte_length < offset)
	{
		return 0;
	}
	
	//	How many data blocks have been read.
	uint32_t db_index = offset/BLOCK_SIZE;
	uint32_t db_first = 1;
	
	//	Iterate through the data blocks until bytes_read >= length or end is reached
	while( (bytes_read < length) && (bytes_read+offset < my_inode->byte_length) )
	{
		//	Find data block from inode data block number.
		data_block_t * data_block = (data_block_t *)(data_head + (my_inode->data_blocks[db_index]) * ADDRESSES_PER_BLOCK);
		
		//	Calculate bytes unread and data left to read.
		uint32_t bytes_unread = length - bytes_read;
		uint32_t data_unread = (my_inode->byte_length) - bytes_read - offset;
		
		//	If bytes unread is at least block size and data unread also is at least block size, just read block size.
		if(bytes_unread >= BLOCK_SIZE && data_unread >= BLOCK_SIZE)
		{
			if(db_first)
			{
				memcpy(buf+bytes_read, data_block+offset, BLOCK_SIZE-offset);
				bytes_read += BLOCK_SIZE-offset;
			}
			else
			{
				memcpy(buf+bytes_read, data_block, BLOCK_SIZE);
				bytes_read += BLOCK_SIZE;
			}
		}
		//	If bytes unread is less than data in block, just read bytes unread.
		else if(bytes_unread < data_unread)
		{
			if(bytes_unread < BLOCK_SIZE-offset)
			{
				memcpy(buf+bytes_read, data_block+offset*db_first, bytes_unread);
				bytes_read += bytes_unread;
			}
			else
			{
				memcpy(buf+bytes_read, data_block+offset*db_first, BLOCK_SIZE-offset);
				bytes_read += BLOCK_SIZE-offset;
			}
		}
		//	If bytes unread is more than data in block, just read data left in block.
		else
		{
			memcpy(buf+bytes_read, data_block+offset*db_first, data_unread);
			bytes_read += data_unread;
		}
		
		db_first = 0;
		db_index++;

	}
	
	return bytes_read;

}
