/* paging.c - Sets up paging and virtual memory
 * vim:ts=4 sw=4 noexpandtab
 */
 
#include "paging.h"
#include "lib.h"
#include "x86_desc.h"

/*create a page directory and table pointer*/
//unsigned int page_directory[NUM_ENTRIES] 	__attribute__((aligned(PAGE_SIZE))); /*must align to page size!*/
//unsigned int page_table[NUM_ENTRIES] 		__attribute__((aligned(PAGE_SIZE)));

/* helper functions */
void _directory_init(void);
void _table_init(void);
void _enable_paging(void);


/*main function
 *
 *
 */
void paging_init(void){
															printf("Page Directory: %x\n", page_directory);
	/*create and initialize the page directory (and contents)*/
	_directory_init();

	/* config cr0 and cr3 */
	_enable_paging();
}


/*helper function:
 *
 *
 */
void _directory_init(){

	unsigned int i;

	for(i = 0; i < NUM_ENTRIES; i++){
		page_directory[i] = EMPTY; //zero
	}
	
	/*Map the first two entries*/
	
	/*create and initialize the first page table */			printf("Page Table:  %x\n", page_table);
	_table_init();	

	/*put page table in page directory*/
	page_directory[0] = (unsigned int)page_table;
	page_directory[0] |= SET_RW_P;

	/*map second entry to our 4mb kernel-space*/
	page_directory[1] = (SET_4MB | SET_RW_P); //0x80

}


/*helper function:
 *	create a page table
 *
 */
void _table_init(){

	unsigned int i;
	unsigned int page_address = PAGE_SIZE;	//4096 - the second Page_table entry

		
	/*first 4kb of memory is not present*/
	page_table[0] = 0;
	
	for(i = 1; i < NUM_ENTRIES; i++){
		page_table[i] = page_address | SET_RW_P;
		page_address += PAGE_SIZE;
	}


}


/*helper function:
 *	Enable Paging
 *	Set cr0, cr3, and cr4 as needed
 *	Reference: IA32: 2.5, 3.6.1, 3.7.1
 */
void _enable_paging(){

	/* register value holder */
	unsigned int cr0_temp, cr4_temp;
	cr0_temp = cr4_temp = 0;
	
	/* Strategy:
	 *	Set cr3 > cr4 > cr0
	 */
	
    /*CR3: Set cr3 to base of page directory*/
	asm volatile("movl %0, %%cr3"
				:                       /*no outputs*/
				:"r"(page_directory)    /*inputs*/
				);
        
	/*CR4: Extract register cr4*/							printf("cr4_temp(zero):   %x\n", cr4_temp);
    asm volatile("movl %%cr4, %0"
                :"=r"(cr4_temp)         /*outputs*/
                :                       /*no inputs*/
                );											printf("cr4_temp(after):  %x\n", cr4_temp);
				
	/*CR4: Set PAE(5)=0 then PSE(4)=1 flags*/
    cr4_temp &= 0xffffffdf;									printf("cr4_temp(and):    %x\n", cr4_temp);
	cr4_temp |= 0x00000010;									printf("cr4_temp(or):     %x\n", cr4_temp);
    asm volatile("movl %0, %%cr4"
                :                       /*no outputs*/
                :"r"(cr4_temp)          /*inputs*/
                );
	

	/*CR0: Extract register cr0*/							printf("cr0_temp(zero):  %x\n", cr0_temp);
    asm volatile("movl %%cr0, %0"
                :"=r"(cr0_temp)         /*outputs*/
                :                       /*no inputs*/
                );											printf("cr0_temp(after): %x\n", cr0_temp);
				
	/*CR0: Set PE(0)=1 AND PG(31)=1 flags*/
    cr0_temp |= 0x80000001;									printf("cr0_temp(or):    %x\n", cr0_temp);
    asm volatile("movl %0, %%cr0"
                :                       /*no outputs*/
                :"r"(cr0_temp)          /*inputs*/
                );
	
}
