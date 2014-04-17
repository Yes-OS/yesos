/* file_sys.c - Functions to initialize and interact
 *              with the file system.
 *
 * vim:ts=4 sw=4 noexpandtab
 *
 * TO DO:
 *		-Add functionality:
 *			copy a program image from the disk blocks into continuous physical memory
 *			set up the stack properly and return to user-level
 */

#include "file_sys.h"

/* File operations jump table */
fops_t file_fops = {
	.read  = &file_read,
	.write = &file_write,
	.open  = &file_open,
	.close = &file_close,
};

/* Directory operations jump table */
fops_t dir_fops = {
	.read  = &dir_read,
	.write = &dir_write,
	.open  = &dir_open,
	.close = &dir_close,
};

/*Variables for File_sys functions*/
static index_node_t* node_head;
static data_block_t* data_head;
static boot_block_t * boot_block;
boot_block_t* mbi_val;


/* Sets up the head pointer for the file system
 * Sets up relevant structures and variables
 *
 * Inputs:	pointer to fs head
 * Outputs:	none
 */
void fs_init(void)
{
	boot_block = mbi_val;

	node_head = (index_node_t*)boot_block + 1;
	data_head = (data_block_t*)node_head + boot_block->num_nodes;
}

/*  Read data from specified file into specified buffer
 *  Return amount read.
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes)
{
	file_t *file;
	int32_t ret;

	file = get_file_from_fd(fd);

	/* ensure file is open */
	if (!file || !(file->flags & FILE_OPEN)) {
		return -1;
	}

	ret = read_data(file->inode_ptr, file->file_pos, (uint8_t *)buf, nbytes);
	file->file_pos += ret;

	return ret;
}

/*  Read-only file system.
 *  Not implemented (yet ;] ).
 *  Return -1
 */
int32_t file_write(int32_t fd, const void* buf, int nbytes)
{
	/* silence warnings about unused variables */
	(void)fd; (void)buf; (void)nbytes;
	return -1;
}

/*  Filler function. File system already open
 *  Return 0;
 */
int32_t file_open(const uint8_t *filename)
{
	dentry_t dentry;
	int32_t fd;
	file_t *file;
	int32_t status;

	status = read_dentry_by_name(filename, &dentry);
	if (status) {
		return -1;
	}

	/* find unused descriptor */
	fd = get_unused_fd();
	if (fd < 0) {
		return -1;
	}

	/* grab associated file, should never fail since we were just allocated a FD */
	file = get_file_from_fd(fd);

	/* set up the file */
	file->flags |= FILE_PRESENT | FILE_OPEN;
	file->inode_ptr = dentry.inode_num;
	file->file_pos = 0;
	file->file_op = &file_fops;

	return 0;
}

/*  Filler function. File system does not close
 *  Return 0;
 */
int32_t file_close(int32_t fd)
{
	release_fd(fd);
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
	dentry_t* temp;

	/*start from dentry index 1 because index 0 is current directory*/
	uint8_t i;
	uint32_t entries = boot_block->num_entries;

	uint32_t len_filename;

	for(i = 0; i <= entries ; i++) {
	
		temp = &boot_block->entries[i];

		/* length of the filename in the filesystem */
		len_filename = strlen((int8_t*)temp->file_name);

		/* there are some null entries in the fs; needed or they'll match any string passed */
		if (!len_filename) {
			continue;
		}

		/* we need to compare the given string to the filename on the length of the filename,
		 * otherwise we can pass "sh" and match "shell" */
		if(!strncmp((int8_t*)fname, (int8_t*)temp->file_name, len_filename)) {
			strncpy((int8_t *)dentry->file_name, (int8_t*)temp->file_name, len_filename);
			/* just in case the filename doesn't end in a null character */
			dentry->file_name[len_filename] = '\0';
			dentry->file_type = temp->file_type;
			dentry->inode_num = temp->inode_num;

			return 0;
		}
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
	if(index <= boot_block->num_entries)
	{

		strncpy((int8_t*)dentry->file_name, (int8_t*)boot_block->entries[index].file_name, FILE_NAME_SIZE);
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

	index_node_t* my_inode = &node_head[inode];

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
		data_block_t * data_block = data_head + my_inode->data_blocks[db_index];

		//	Calculate bytes unread and data left to read.
		uint32_t bytes_unread = length - bytes_read;
		uint32_t data_unread = (my_inode->byte_length) - bytes_read - offset;

		//	If bytes unread is at least block size and data unread also is at least block size, just read block size.
		if(bytes_unread >= BLOCK_SIZE && data_unread >= BLOCK_SIZE)
		{
			memcpy(buf+bytes_read, &(data_block->data[offset*db_first]), BLOCK_SIZE-offset*db_first);
			bytes_read += BLOCK_SIZE-offset*db_first;
		}
		//	If bytes unread is less than data in block, just read bytes unread.
		else if(bytes_unread < data_unread)
		{
			if(bytes_unread < BLOCK_SIZE-offset*db_first)
			{
				memcpy(buf+bytes_read, &(data_block->data[offset*db_first]), bytes_unread);
				bytes_read += bytes_unread;
			}
			else
			{
				memcpy(buf+bytes_read, &(data_block->data[offset*db_first]), BLOCK_SIZE-offset*db_first);
				bytes_read += BLOCK_SIZE-offset*db_first;
			}
		}
		//	If bytes unread is more than data in block, just read data left in block.
		else
		{
			memcpy(buf+bytes_read, &(data_block->data[offset*db_first]), data_unread);
			bytes_read += data_unread;
		}

		db_first = 0;
		db_index++;

	}

	return bytes_read;

}

/*  Dir_read for the dir fops_table
 *  
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes)
{
	file_t *file;
	int32_t ret;
	dentry_t dentry;

	file = get_file_from_fd(fd);

	/* ensure file is open */
	if (!file || !(file->flags & FILE_OPEN)) {
		return -1;
	}
	
	ret = read_dentry_by_index(file->inode_ptr, &dentry);
	strncpy((int8_t*)buf, (int8_t*)dentry.file_name, FILE_NAME_SIZE);
	file->file_pos += ret;

	return ret;
}

/*  Read-only file system.
 * 
 *  Return -1
 */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes)
{
	(void)fd; (void)buf; (void)nbytes;
	return -1;
}

/* Dir_open for fops table
 *  
 */
int32_t dir_open(const uint8_t *filename)
{
	dentry_t dentry;
	int32_t fd;
	file_t *file;
	int32_t status;

	status = read_dentry_by_name(filename, &dentry);
	if (status) {
		return -1;
	}

	/* find unused descriptor */
	fd = get_unused_fd();
	if (fd < 0) {
		return -1;
	}

	/* grab associated file, should never fail since we were just allocated a FD */
	file = get_file_from_fd(fd);

	/* set up the file */
	file->flags |= FILE_PRESENT | FILE_OPEN;
	file->inode_ptr = dentry.inode_num;
	file->file_pos = 0;
	file->file_op = &dir_fops;

	return fd;
}

/*  Filler function. Directory does not close
 *  Return 0;
 */
int32_t dir_close(int32_t fd)
{
	release_fd(fd);
	return 0;
}

/*
 * read a passed file
 * get EIP
 * copy file to physical memory using its own virtual space
 *
 * returns 0 on success (for now)
 *		  -1 on failure
 */
uint32_t file_loader(dentry_t* file, uint32_t* EIP){

	uint32_t bytes_remaining = node_head[file->inode_num].byte_length;
	uint32_t curEIP, temp_read;
	uint32_t buf_length = 128;
	uint32_t bytes_read = 0;
	uint8_t file_buf[buf_length];

	/* populate file's page directory
	 * 		use memcpy to new page in new page directory
	 * 		should fill one page table (no more than 4MB a task)
	 */
	while(bytes_remaining > 0) {
		temp_read = read_data(file->inode_num, bytes_read, file_buf, buf_length);
		memcpy((uint32_t*)(USER_MEM + EXEC_OFFSET + bytes_read), file_buf, temp_read);
		bytes_read += temp_read;
		bytes_remaining -= temp_read;
	}

	/*EIP is bytes 24-27 of executable*/
	curEIP = *(uint32_t*)(USER_MEM + EXEC_OFFSET + 24);
	if(curEIP < USER_MEM + EXEC_OFFSET || curEIP > USER_MEM + MB_4_OFFSET) {
		printf("Invalid EIP: Not an executable\n");
		return -1;
	}

	*EIP = curEIP;




	return 0;
}
