/* paging.c - Sets up paging and virtual memory
 * vim:ts=4 sw=4 noexpandtab
 */

#include "paging.h"
#include "lib.h"
#include "vga.h"
#include "x86_desc.h"
#include "proc.h"

/* +1 is for the kernel's Page Directory */
pd_t page_directories[MAX_PROCESSES + 1] __attribute__((aligned(PAGE_SIZE)));

/* For Real Video Memory */
pt_t video_memories[MAX_PROCESSES + 1] __attribute__((aligned(PAGE_SIZE)));

/* For mapping user video memory */
pt_t user_video_mems[NUM_TERMS] __attribute__((aligned(PAGE_SIZE)));

/* only used by one process at a time for temporary operations, like
 * swapping video memory */
pt_t temp_table __attribute__((aligned(PAGE_SIZE)));

/* define some actions to set/clear status bits */

/* sets the 31 bit of CR0 */
/* enables memory paging */
#define set_pg_flag()                               \
	do {                                            \
		asm volatile (                              \
				"movl    %%cr0, %%eax\n             \
				 orl     $0x80000000, %%eax\n       \
				 movl    %%eax, %%cr0"              \
				: : : "eax", "cc", "memory"         \
			);                                      \
	} while (0)

/* sets the 4 bit of CR4 */
/* enables page size extensions, allowing 4 MB pages */
#define set_pse_flag()                         \
	do {                                       \
		asm (                                  \
				"movl    %%cr4, %%eax\n        \
				 orl     $0x00000010, %%eax\n  \
				 movl    %%eax, %%cr4"         \
				: : : "eax", "cc"              \
			);                                 \
	} while (0)

/* clears the 5 bit of CR4 */
/* disables physical address extension */
#define clr_pae_flag()                         \
	do {                                       \
		asm (                                  \
				"movl    %%cr4, %%eax\n        \
				 andl    $0xFFFFFFDF, %%eax\n  \
				 movl    %%eax, %%cr4"         \
				: : : "eax", "cc"              \
			);                                 \
	} while (0)

/* helper functions */
static void clear_page_dir(pd_t* directory);
static void clear_page_table(pt_t* table);
static void install_pages();
static void install_kernel_page(pd_t *page_directory);
static void install_user_page(uint32_t index, pd_t *page_directory);
static void map_video_mem(const vid_mem_t *vidmem, const void *virt_addr, pd_t *proc_pd, pt_t *page_table);

/* define some empty values, useful for initialization */
static const pte_t empty_page_entry = {{.val = 0UL}};
static const pde_t empty_dir_entry = {{.val = 0UL}};

/* setup kernel page */
static void install_kernel_page(pd_t *page_directory)
{
		pde_t kernel_mem = empty_dir_entry;

		kernel_mem.present = 1;
		kernel_mem.read_write = 1;
		kernel_mem.user_supervisor = 0;
		kernel_mem.page_size = 1;
		kernel_mem.page_base_addr_4mb = PAGE_BASE_ADDR_4MB(KERNEL_MEM);

		page_directory->entry[PAGE_DIR_IDX(KERNEL_MEM)] = kernel_mem;
}

static void install_user_page(uint32_t index, pd_t *page_directory)
{
	pde_t user_mem = empty_dir_entry;

	user_mem.present = 1;
	user_mem.read_write = 1;
	user_mem.user_supervisor = 1;
	user_mem.page_size = 1;
	user_mem.page_base_addr_4mb = PAGE_BASE_ADDR_4MB(KERNEL_MEM+index*MB_4_OFFSET);

	page_directory->entry[PAGE_DIR_IDX(USER_MEM)] = user_mem;
}

void install_user_vid_mem(pd_t *page_directory, pt_t *user_vid_mem_table)
{
	int i;

	/* setup first page directory */
	pde_t vid_mem_dir = empty_dir_entry;

	vid_mem_dir.present = 1;
	vid_mem_dir.read_write = 1;
	vid_mem_dir.user_supervisor = 1;
	vid_mem_dir.pt_base_addr = PAGE_BASE_ADDR((uint32_t)&user_vid_mem_table);

	page_directory->entry[PAGE_DIR_IDX(USER_VID)] = vid_mem_dir;

	/* setup video memory */
	pte_t video_mem_temp = empty_page_entry;
	for (i = 0; i < 16; i++) {
		video_mem_temp.present = 1;
		video_mem_temp.read_write = 1;
		video_mem_temp.user_supervisor = 1;
		/* every 4kb page */
		video_mem_temp.page_base_addr = PAGE_BASE_ADDR(VIDEO + i * 0x1000);
		user_vid_mem_table->entry[PAGE_TABLE_IDX(USER_VID + i * 0x1000)] = video_mem_temp;
	}
}

static void map_video_mem(const vid_mem_t *vidmem, const void *virt_addr, pd_t *proc_pd, pt_t *page_table)
{
	int32_t i;

	clear_page_table(page_table);
	for (i = 0; i < (int32_t)(sizeof vidmem->data)/PAGE_SIZE; i++) {
		pte_t temp_entry = empty_page_entry;

		temp_entry.present = 1;
		temp_entry.read_write = 1;
		temp_entry.user_supervisor = 1;
		temp_entry.page_base_addr = PAGE_BASE_ADDR((uint32_t)vidmem + i * PAGE_SIZE);

		page_table->entry[PAGE_TABLE_IDX((uint32_t)virt_addr + i * PAGE_SIZE)] = temp_entry;
	}

	{
		pde_t temp_entry = empty_dir_entry;

		temp_entry.present = 1;
		temp_entry.read_write = 0;
		temp_entry.user_supervisor = 1;
		temp_entry.pt_base_addr = PAGE_BASE_ADDR((uint32_t)page_table);

		proc_pd->entry[PAGE_DIR_IDX((uint32_t)virt_addr)] = temp_entry;
	}
}

/*
 * called from the scheduler to swap the running process's video memory
 * into fake video memory
 */
int32_t switch_to_fake_video_memory()
{
	pcb_t *pcb;
	pd_t *proc_pd;
	screen_t *screen;
	uint32_t flags;
	vid_mem_t *fake;

	cli_and_save(flags);
	pcb = get_proc_pcb();
	if (!pcb) {
		return -1;
	}

	proc_pd = pcb->page_directory;
	fake = get_proc_fake_vid_mem();

	screen = get_screen_ctx();
	if (!screen) {
		return -1;
	}

	/* sanity check, don't swap video memory if the process is already
	 * operating with fake video memory */
	if (screen->video == fake) {
		return -1;
	}

	/* otherwise, set the screen to point to fake memory */
	screen->video = fake;

	/* temporarily map fake video memory */
	map_video_mem(fake, fake, proc_pd, &temp_table);

	/* flush tlb */
	set_pdbr(proc_pd);

	/* now actually copy to fake video memory */
	screen_save(screen);

	/* unmap fake video memory */
	proc_pd->entry[PAGE_DIR_IDX((uint32_t)fake)].present = 0;
	map_video_mem(fake, (void *)VIDEO, proc_pd, &video_memories[terminal_num]);

	if (pcb->has_video_mapped) {
		map_video_mem(fake, (void *)USER_VID, proc_pd, &user_video_mems[pcb->pid]);
	}

	/* flush tlb again */
	set_pdbr(proc_pd);

	restore_flags(flags);
	return 0;
}

/*
 * called from the scheduler to swap the running process's video memory
 * from fake video memory into real video memory
 */
int32_t switch_from_fake_video_memory()
{
	pcb_t *pcb;
	pd_t *proc_pd;
	screen_t *screen;
	uint32_t flags;
	vid_mem_t *fake;

	cli_and_save(flags);
	pcb = get_proc_pcb();
	if (!pcb) {
		return -1;
	}

	screen = get_screen_ctx();
	if (!screen) {
		return -1;
	}

	proc_pd = pcb->page_directory;
	fake = get_proc_fake_vid_mem();

	/* sanity check, don't swap video memory if the process is already
	 * operating with fake video memory */
	if (screen->video == fake) {
		return -1;
	}

	/* temporarily map fake video memory */
	map_video_mem(fake, fake, proc_pd, &temp_table);

	/* flush tlb */
	set_pdbr(proc_pd);

	/* now actually copy to fake video memory */
	screen_restore(screen);

	/* unmap fake video memory */
	proc_pd->entry[PAGE_DIR_IDX((uint32_t)fake)].present = 0;
	map_video_mem((void *)VIDEO, (void *)VIDEO, proc_pd, &video_memories[terminal_num]);
	screen->video = (vid_mem_t *)VIDEO;

	if (pcb->has_video_mapped) {
		map_video_mem((void *)USER_VID, (void *)USER_VID, proc_pd, &user_video_mems[pcb->pid]);
	}

	/* flush tlb again */
	set_pdbr(proc_pd);

	restore_flags(flags);
	return 0;
}

/* initializes paging */
void paging_init(void)
{
	install_pages();
}

/* clears the page directory for the kernel */
static void clear_page_dir(pd_t* directory)
{
	int i;

	/* clear page directory */
	for (i = 0; i < NUM_ENTRIES; i++) {
		/* install an empty entry */
		directory->entry[i] = empty_dir_entry;
	}
}

/* clears the first page table */
static void clear_page_table(pt_t* table)
{
	int i;

	/* clear first page table */
	for (i = 0; i < NUM_ENTRIES; i++) {
		/* install an empty entry */
		table->entry[i] = empty_page_entry;
	}
}

/* installs the 4MB page for the kernel, and maps 64k of video memory */
static void install_pages()
{
	int i;

	for(i = 0; i < MAX_PROCESSES + 1; i++) {
		clear_page_dir(&page_directories[i]);
		clear_page_table(&video_memories[i]);
		install_kernel_page(&page_directories[i]);
		map_video_mem((void *)VIDEO, (void *)VIDEO, &page_directories[i], &video_memories[i]);
		if (i > 0) {
			install_user_page(i, &page_directories[i]);
		}
	}

	fake_video_mem = (vid_mem_t *)(KERNEL_MEM + MB_4_OFFSET * (MAX_PROCESSES + 1));

	/* set up registers */
	clr_pae_flag();
	set_pse_flag();
	set_pdbr(&page_directories[0]);

	/* enable paging */
	set_pg_flag();
}
