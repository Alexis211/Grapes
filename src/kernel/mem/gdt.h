#ifndef DEF_GDT_H
#define DEF_GDT_H

#include <types.h>

/*	The GDT is one of the x86's descriptor tables. It is used for memory segmentation.
	Here, we don't use segmentation to separate processes from one another (this is done with paging).
	We only use segmentation to make the difference between kernel mode (ring 3) and user mode (ring 0) */

/* 	One entry of the table */
struct gdt_entry {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
} __attribute__((packed));

/*	Structure defining the whole table : address and size (in bytes). */
struct gdt_ptr {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

/*	The TSS is used for hardware multitasking.
	We don't use that, but we still need a TSS so that user mode process exceptions
	can be handled correctly by the kernel.	*/
struct tss_entry {
	uint32_t prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
	uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
	uint32_t ss0;        // The stack segment to load when we change to kernel mode.
	uint32_t esp1;       // Unused...
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;         // The value to load into ES when we change to kernel mode.
	uint32_t cs;         // The value to load into CS when we change to kernel mode.
	uint32_t ss;         // The value to load into SS when we change to kernel mode.
	uint32_t ds;         // The value to load into DS when we change to kernel mode.
	uint32_t fs;         // The value to load into FS when we change to kernel mode.
	uint32_t gs;         // The value to load into GS when we change to kernel mode.
	uint32_t ldt;        // Unused...
	uint16_t trap;
	uint16_t iomap_base;
} __attribute__((packed));

void gdt_init();
void gdt_setKernelStack(uint32_t esp0);

#endif

