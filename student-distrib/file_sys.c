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

#include "lib.h"
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
static boot_block_t* boot_block;


/* Sets up the head pointer for the file system
 * Sets up relevant structures and variables
 *
 * Inputs:	pointer to fs head
 * Outputs:	none
 */
void fs_init(boot_block_t* boot_val)
{
	boot_block = boot_val;

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
	file->flags |= FILE_OPEN;
	file->inode_ptr = dentry.inode_num;
	file->file_pos = 0;
	file->file_op = &file_fops;

	return fd;
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

		/* we need to compare on the max length of a file, or else we get weird matching results */
		if(!strncmp((int8_t*)fname, (int8_t*)temp->file_name, FILE_NAME_SIZE)) {
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
	//Check for non-existant file or invalid index
	if(index <= boot_block->num_entries) {
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
	int32_t db_idx;
	int32_t b_read;
	int32_t b_rem;
	int32_t round_read;
	data_block_t *p_db;
	index_node_t *p_inode;
	uint8_t *data;

	/* verify bounds */
	if (inode > boot_block->num_nodes) {
		return -1;
	}

	p_inode = node_head + inode;

	if (p_inode->byte_length < offset) {
		return -1;
	}

	/* compute number of bytes to read */
	b_rem = min(p_inode->byte_length - offset, length);

	/* compute first datablock */
	db_idx = offset / BLOCK_SIZE;
	p_db = data_head + p_inode->data_blocks[db_idx];
	data = p_db->data + offset % BLOCK_SIZE;

	b_read = 0;
	while (b_rem > 0) {
		/* compute number of bytes to read this iteration */
		round_read = min(b_rem, p_db->data + BLOCK_SIZE - data);

		/* copy data into buffer */
		memcpy(buf+b_read, data, round_read);
		b_read += round_read;
		b_rem -= round_read;

		/* get next block */
		p_db = data_head + p_inode->data_blocks[++db_idx];
		data = p_db->data;
	}

	return b_read;
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

	ret = read_dentry_by_index(file->file_pos, &dentry);
	if (ret) {
		/* at the end of the directory */
		return 0;
	}

	strncpy((int8_t*)buf, (int8_t*)dentry.file_name, nbytes);

	/* just in case */
	((int8_t*)buf)[nbytes-1] = '\0';

	/* increment directory position */
	file->file_pos += 1;

	return strlen((int8_t*)dentry.file_name);
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
	file->flags |= FILE_OPEN;
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
		return -1;
	}

	*EIP = curEIP;

	return 0;
}
