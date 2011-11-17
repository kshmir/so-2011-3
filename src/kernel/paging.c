// Declare the page directory and a page table, both 4kb-aligned



typedef struct page_entry {
	unsigned int data[1024];
} page_dir_t, page_entry_t;

page_entry_t global_entries[1200];
page_dir_t 	 * kernel_pdir;
page_entry_t * kernel_pentries;

#define aligned(dir) ((((int)dir + 4095)>>12)<<12)

void init_paging() {
	kernel_pdir = (page_dir_t *) aligned(&global_entries[1]);
	kernel_pentries = (page_entry_t *) aligned(&global_entries[3]);

	
	// Pointers to the page directory and the page table
	void *kernelpagedirPtr = kernel_pdir;
	void *lowpagetablePtr = kernel_pentries;
	int k = 0, j = 0;

	for(; j < 1024; ++j)	{
		// Counts from 0 to 1023 to...
		for (k = 0; k < 1024; k++)	{
			kernel_pentries[j].data[k] = (j * 4096 * 1024) | (k * 4096) | 0x3;	// ...map the first 4MB of memory into the page table...
		}
		if (j < 1023) {
			kernel_pdir->data[j] = (int)kernel_pentries + (j % 16) * 4096  | 0x3;
		} else {
			kernel_pdir->data[j] = 0;
		}
	}

	// kernel_pdir->data[0] = ((int)kernel_pentries) | 0x3;
	// kernel_pdir->data[1] = ((int)kernel_pentries) | 0x3;
	// kernel_pdir->data[2] = ((int)kernel_pentries) | 0x3;
	// kernel_pdir->data[3] = ((int)kernel_pentries) | 0x3;
	// kernel_pdir->data[4] = ((int)kernel_pentries) | 0x3;
	// kernel_pdir->data[5] = ((int)kernel_pentries) | 0x3;



	// Copies the address of the page directory into the CR3 register and, finally, enables paging!

	asm volatile (	"mov %0, %%eax\n"
			"mov %%eax, %%cr3\n"
			"mov %%cr0, %%eax\n"
			"orl $0x80000000, %%eax\n"
			"mov %%eax, %%cr0\n" :: "m" (kernelpagedirPtr));
	*(char*)(0x4b80a2) = 'b';

}