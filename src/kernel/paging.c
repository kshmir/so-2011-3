// Declare the page directory and a page table, both 4kb-aligned

#include "paging.h"

typedef struct page_entry {
	unsigned int data[1024];
} page_dir_t, page_entry_t, page_t;

int 		 used_pages[1024];
int 		 used_entries[64];
page_t       stack_pages[1125];
page_entry_t stackd_entries[165];
page_entry_t global_entries[32];
page_entry_t * aligned_st_pages;
page_entry_t * aligned_st_entries;

page_dir_t 	 * kernel_pdir;
page_entry_t * kernel_pentries;

#define aligned(dir) ((((int)dir + 4095)>>12)<<12)

void init_paging() {
	aligned_st_pages = (page_t *) aligned(&stack_pages[1]);
	aligned_st_entries = (page_entry_t *) aligned(&stackd_entries[1]);
	kernel_pdir = (page_dir_t *) aligned(&global_entries[1]);
	kernel_pentries = (page_entry_t *) aligned(&global_entries[3]);
	
	// Pointers to the page directory and the page table
	void *kernelpagedirPtr = kernel_pdir;
	void *lowpagetablePtr = kernel_pentries;
	int k = 0, j = 0, i = 0;
	
	for(i = 0; i < 1024; ++i) {
		used_pages[i] = 0;
	}
	
	for(i = 0; i < 64; ++i) {
		used_entries[i] = 0;
	}

	for(; j < 1024; ++j)	{
		// Counts from 0 to 1023 to...
		for (k = 0; k < 1024; k++)	{
			kernel_pentries[j % 16].data[k] = ((j % 16) * 4096 * 1024) | (k * 4096) | 0x7;	// ...map the first 4MB of memory into the page table...
		}
		
		kernel_pdir->data[j] = (int)kernel_pentries + (j % 16) * 4096  | 0x7;
	}

	// Copies the address of the page directory into the CR3 register and, finally, enables paging!

	asm volatile ("mov %0, %%eax\n"
			"mov %%eax, %%cr3\n"
			"mov %%cr0, %%eax\n"
			"orl $0x80000000, %%eax\n"
			"mov %%eax, %%cr0\n" :: "m" (kernelpagedirPtr));
			
}

int k = 0, j = 0, i = 0;
page_entry_t * get_stack_entry() {
 	i = 0;
	k = 0;
	for(i = 0; i < 64; ++i)
	{
		if(!used_entries[i])
		{
			used_entries[i] = 1;
			for(k = 0; k < 1024; ++k)	{
				aligned_st_entries[i].data[k] = 0;
			}
			return &aligned_st_entries[i]; 
		}
	}
}

page_t * get_stack_page() {
	i = 0;
	k = 0;
	for(i = 0; i < 1024; ++i)
	{
		if(!used_pages[i])
		{
			used_pages[i] = 1;
			for(k = 0; k < 1024; ++k)
			{
				aligned_st_pages[i].data[k] = 0;
			}
			return &aligned_st_pages[i];
		}
	}
}

void release_stack_page(page_t * page) {
	int index = (page - aligned_st_pages) / 4096;
	used_pages[index] = 0;
}

void release_stack_entry(page_t * entry) {
	int index = (entry - aligned_st_entries) / 4096;
	used_entries[index] = 0;
}

int index = 0;

void add_process_stack(Process * p) {
	page_entry_t * pentry = (page_entry_t *) p->stacke;
	page_t * pstack  = get_stack_page();
	p->stack_index++;
	pentry->data[1023 - p->stack_index] = (int) pstack | 0x3;
}

int release_process_stack(Process * p) {
	if(p->stack_index > 1)
	{
		page_entry_t * pentry = (page_entry_t *) p->stacke;
		release_stack_page((page_t *)(pentry->data[1023 - p->stack_index]));
		p->stack_index--;
		return 0;
	} else {
		return 1;
	}
}




void set_proc_stack(Process * p) {

	if (kernel_rd()) {
		int esp = sched_pindex(p);
		*(char*)(0xb8510) = esp % 10 + '0';
		*(char*)(0xb850e) = (esp / 10) % 10 + '0';
		*(char*)(0xb850c) = (esp / 100) % 10 + '0';
		*(char*)(0xb850a) = (esp / 1000) % 10 + '0';
		*(char*)(0xb8508) = (esp / 10000) % 10 + '0';
		*(char*)(0xb8506) = (esp / 100000) % 10 + '0';
		*(char*)(0xb8504) = (esp / 1000000) % 10 + '0';
		*(char*)(0xb8502) = (esp / 10000000) % 10 + '0';
		*(char*)(0xb8500) = (esp / 100000000) % 10 + '0';
	}
	
	aligned_st_pages         = (page_t *) aligned(&stack_pages[1]);
	aligned_st_entries       = (page_entry_t *) aligned(&stackd_entries[1]);
	kernel_pentries          = (page_entry_t *) aligned(&global_entries[3]);

	if(!p->stacke) {
		page_entry_t * pentry = get_stack_entry();

		for(j = 0; j < 1024; ++j)	{
			pentry->data[j] = 0x3;
		}

		page_t * pstack  = get_stack_page();

		p->stacke = (void *)((int)p->stacke | (int)pentry);
		p->stackp = (void *) (0xFFFFFFFF - 4096 - 4096 * 1024 * sched_pindex(p)); // All stacks start here.

		kernel_pdir->data[1023 - sched_pindex(p)] = (int) pentry | 0x3;

		pentry->data[1023] = (int) pstack  | 0x3;

		p->stack_index = 0;
		p->loaded = 1;
	}

	// pentry->data[1020] = (int) pstack4 | 0x3;
	// pentry->data[1019] = (int) pstack5 | 0x3;
	// pentry->data[1018] = (int) pstack6 | 0x3;
	// pentry->data[1017] = (int) pstack7 | 0x3;
}