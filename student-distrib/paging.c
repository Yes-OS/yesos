/* paging.c - Sets up paging and virtual memory
 * vim:ts=4 noexpandtab
 */
 
#include "paging.h"
#include "lib.h"

/*create a page directory*/
	
//unsigned int* page_directory __attribute__((aligned(PAGE_SIZE)));
unsigned int page_directory[NUM_ENTRIES];
unsigned int* page_table;

/* helper functions */
void _directory_init(void);
void _table_init(void);
void _enable_paging(void);


/*main function
 *
 *
 */
void paging_init(void){

	/*create and initialize the page directory */
	printf("Page Directory(after): %x\n", page_directory);
	_directory_init();
	
	/*create and initialize the first page table */
	printf("Page Table(before): %x\n", page_table);
	_table_init();
	printf("Page Table(after): %x\n", page_table);

	/* config cr0 and cr3 */
	//_enable_paging();
}


/*helper function:
 *
 *
 */
void _directory_init(){

	unsigned int i;

	for(i = 0; i < NUM_ENTRIES; i++){
		page_directory[i] = SET_RW;
	}

}


/*helper function:
 *	create a page table
 *
 */
void _table_init(){

	unsigned int i;
	unsigned int page_address = PAGE_SIZE;	
    page_table = page_directory + NUM_ENTRIES;

		
	/*first 4kb of memory is not present*/
	page_table[0] = 0;
	
	for(i = 1; i < NUM_ENTRIES; i++){
		page_table[i] = page_address | SET_RW_P;
		page_address += PAGE_SIZE;
	}

	/*put page table in page directory*/
	
	page_directory[0] = (unsigned int)page_table;
	page_directory[0] |= SET_RW_P;

}


/*helper function:
 *	Enable Paging using cr3 and cr0
 *
 */
void _enable_paging(){

	unsigned int cr0_temp;
    
    /*Set cr3 to base of page directory*/
	asm volatile("movl %0, %%cr3"
				:                       /*no outputs*/
				:"b"(page_directory)    /*inputs*/
				);
        
    /*Extract register cr0, set the paging bit*/
    cr0_temp = 0;												printf("cr0_temp(zero): %d\n", cr0_temp);
    asm volatile("movl %%cr0, %0"
                :"=b"(cr0_temp)         /*outputs*/
                :                       /*no inputs*/
                );												printf("cr0_temp(after): %x\n", cr0_temp);
    cr0_temp |= 0x80000001;										printf("cr0_temp(or): %x\n", cr0_temp);
    asm volatile("movl %0, %%cr0"
                :                       //no outputs
                :"b"(cr0_temp)          //inputs
                );
	
}
