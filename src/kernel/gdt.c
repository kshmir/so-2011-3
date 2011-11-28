#include "gdt.h"

#pragma pack(1)

struct SegmentDescriptor {
    short limit;
    short base_l;
    char base_m;
    char access;
    char attribs;
    char base_h;
} __attribute__((packed));

static struct TaskState ts;
static struct SegmentDescriptor gdt[6];

struct GDTR {
    short limit;
    int base;
} __attribute__((packed));

static void setupGDTEntry(int num, int base, int limit, short access, short gran) {
   gdt[num].base_l      = (base & 0xFFFF);
   gdt[num].base_m      = (base >> 16) & 0xFF;
   gdt[num].base_h      = (base >> 24) & 0xFF;
   gdt[num].limit       = (limit & 0xFFFF);
   gdt[num].attribs     = ((limit >> 16) & 0x0F) | (gran & 0xF0);
   gdt[num].access      = access;
}

void setupGDT(void) {

    struct GDTR gdtr;
    struct TaskState* ts = task_create_tss();

    setupGDTEntry(0, 0, 0, 0, 0);
    setupGDTEntry(1, 0, 0x000FFFFF, 0x9A, 0xC0);
    setupGDTEntry(2, 0, 0x000FFFFF, 0x92, 0xC0);
    setupGDTEntry(3, 0, 0x000FFFFF, 0xFC, 0xC0);
    setupGDTEntry(4, 0, 0x000FFFFF, 0xF2, 0xC0);
    setupGDTEntry(5, (int) ts, (int) (ts + 1) + 1, 0x89, 0);

    gdtr.limit = 6 * sizeof(struct SegmentDescriptor) - 1;
    gdtr.base = (int) gdt;

    __asm__ volatile("lgdt (%%eax)"::"A"(&gdtr):);
    __asm__ volatile("ltr %%ax"::"a"(TASK_STATE_SEGMENT):);


}

unsigned int gdt_page_address(void) {
    return (unsigned int) gdt;
}



extern int kernel_stack;

struct TaskState* task_create_tss(void) {
    ts.iopb = sizeof(struct TaskState);

    ts.cs = KERNEL_CODE_SEGMENT;
    ts.ds = KERNEL_DATA_SEGMENT;
    ts.es = KERNEL_DATA_SEGMENT;
    ts.fs = KERNEL_DATA_SEGMENT;
    ts.gs = KERNEL_DATA_SEGMENT;
    ts.ss = KERNEL_DATA_SEGMENT;
    ts.ss0 = KERNEL_DATA_SEGMENT;
	ts.esp0 = kernel_stack;

    return &ts;
}

void set_user_mode(){ 
// Set up a stack structure for switching to user mode. 
		asm volatile(
		"cli;"
		"mov $0x23, %ax;"
		"mov %ax, %ds;"
		"mov %ax, %es;"
		"mov %ax, %fs;"
		"mov %ax, %gs;" 
		"mov %esp, %eax;"
		"pushl $0x23;"
		"pushl %eax;"
		"pushf;"
		"mov $0x200, %eax;"
		"push %eax;"
		"pushl $0x1B;"
		"push $1f;"
		"iret;"
		"1:"); 
}

unsigned int task_page_address(void) {
    return (unsigned int) &ts;
}

