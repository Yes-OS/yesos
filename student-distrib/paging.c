/* paging.c - Sets up paging and virtual memory
 * vim:ts=4 noexpandtab
 */
 
#include "paging.h"

/*create a page directory*/
	
unsigned int* page_directory __attribute__((aligned(PAGE_SIZE)));
unsigned int* page_table;


void paging_init(void)
{
	unsigned int page_address = PAGE_SIZE;	
    page_table = page_directory + NUM_ENTRIES;
	unsigned int i, cr0_temp;

	for(i = 0; i < NUM_ENTRIES; i++)
	{
		page_directory[i] = SET_RW;
	}
	
	/*create a page table*/
		
	/*first 4kb of memory is not present*/
	page_table[0] = 0;
	
	for(i = 1; i < NUM_ENTRIES; i++)
	{
		page_table[i] = page_address | SET_RW_P;
		page_address += PAGE_SIZE;
	}

	/*put page table in page directory*/
	
	page_directory[0] = (unsigned int)page_table;
	page_directory[0] |= SET_RW_P;
	
	/*Enable Paging using cr3 and cr0*/
    
    /*Set cr3 to base of page directory*/
	asm volatile("movl %0, %%cr3"
				:                       /*no outputs*/
				:"q"(page_directory)    /*inputs*/
				);
        
    /*Extract register cr0, set the paging bit*/
    cr0_temp = 0;
    asm volatile("movl %%cr0, %0"
                :"=q"(cr0_temp)         /*outputs*/
                :                       /*no inputs*/
                );
    cr0_temp |= 0x80000000;
    asm volatile("movl %0, %%cr0"
                :                       /*no outputs*/
                :"q"(cr0_temp)          /*inputs*/
                );
                
}
