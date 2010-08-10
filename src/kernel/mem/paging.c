#include "paging.h"
#include <bitset.h>
#include <stdlib.h>
#include <core/monitor.h>
#include "mem.h"
#include "seg.h"
#include <core/sys.h>
#include <task/task.h>

static struct bitset frames;

struct page_directory *kernel_pagedir, *current_pagedir;

/**************************		PHYSICAL MEMORY ALLOCATION	************************/

/*	Allocates a page of physical memory. */
uint32_t frame_alloc() {
	uint32_t free = bitset_firstFree(&frames);
	bitset_set(&frames, free);
	return free;
}

void frame_free(uint32_t id) {
	bitset_clear(&frames, id);
}

/*************************		PAGING INITIALIZATION		*****************************/

/*	This function creates the kernel page directory. It must be called before the GDT is loaded.
	It maps 0xE0000000+ to the corresponding physical kernel code, but it also maps
	0x00000000+ to that code because with the false GDT we set up in loader_.asm,
	the code will be looked for at the beginning of the memory. Only when the real GDT is loaded
	we can de-allocate pages at 0x00000000 ; this is done by paging_cleanup. */
void paging_init(size_t totalRam) {
	uint32_t i;

	frames.size = totalRam / 0x1000;
	frames.bits = kmalloc(INDEX_FROM_BIT(frames.size));

	kernel_pagedir = kmalloc(sizeof(struct page_directory));
	kernel_pagedir->mappedSegs = 0;
	kernel_pagedir->tablesPhysical = kmalloc_page(&kernel_pagedir->physicalAddr);
	for (i = 0; i < 1024; i++) {
		kernel_pagedir->tables[i] = 0;
		kernel_pagedir->tablesPhysical[i] = 0;
	}

	for (i = 0xE0000000; i < mem_placementAddr; i += 0x1000) {
		page_map(pagedir_getPage(kernel_pagedir, i, 1), frame_alloc(), 0, 0);
	}
	for (i = 0; i < (mem_placementAddr - 0xE0000000) / 0x100000; i++) {
		kernel_pagedir->tablesPhysical[i] = kernel_pagedir->tablesPhysical[i + 896];
		kernel_pagedir->tables[i] = kernel_pagedir->tables[i + 896];
	}

	monitor_write("{PD: ");
	monitor_writeHex(kernel_pagedir->physicalAddr);
	pagedir_switch(kernel_pagedir);
	monitor_write("} [Paging] ");
}

/*	De-allocates pages at 0x00000000 where kernel code was read from with the GDT from loader_.asm. */
void paging_cleanup() {
	uint32_t i;
	for (i = 0; i < (mem_placementAddr - 0xE0000000) / 0x100000; i++) {
		kernel_pagedir->tablesPhysical[i] = 0;
		kernel_pagedir->tables[i] = 0;
	}
	monitor_write("[PD Cleanup] ");
}

/*************************		PAGING EVERYDAY USE		*****************************/

/*	Switch to a page directory. Can be done if we are sure not to be interrupted by a task switch.
	Example use for cross-memory space writing in linker/elf.c */
void pagedir_switch(struct page_directory *pd) {
	current_pagedir = pd;
	asm volatile("mov %0, %%cr3" : : "r"(pd->physicalAddr));
	uint32_t cr0;
	asm volatile("mov %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0" : : "r"(cr0));
}

/*	Creates a new page directory for a process, and maps the kernel page tables on it. */
struct page_directory *pagedir_new() {
	uint32_t i;

	struct page_directory *pd = kmalloc(sizeof(struct page_directory));
	pd->tablesPhysical = kmalloc_page(&pd->physicalAddr);
	pd->mappedSegs = 0;

	for (i = 0; i < 1024; i++) {
		pd->tables[i] = 0; pd->tablesPhysical[i] = 0;
	}

	for (i = 896; i < 1024; i++) {
		pd->tables[i] = kernel_pagedir->tables[i];
		pd->tablesPhysical[i] = kernel_pagedir->tablesPhysical[i];
	}

	return pd;
}

/*	Deletes a page directory, cleaning it up. */
void pagedir_delete(struct page_directory *pd) {
	uint32_t i;
	//Unmap segments
	while (pd->mappedSegs != 0) seg_unmap(pd->mappedSegs);
	//Cleanup page tables
	for (i = 0; i < 896; i++) {
		kfree_page(pd->tables[i]);
	}
	kfree_page(pd->tablesPhysical);
	kfree(pd);
}

/*	Handle a paging fault. First, looks for the corresponding segment.
	If the segment was found and it handles the fault, return normally.
	Else, display informatinos and return an error. */
uint32_t paging_fault(struct registers *regs) {
	size_t addr;
	struct segment_map *seg = 0;
	asm volatile("mov %%cr2, %0" : "=r"(addr));

	seg = current_pagedir->mappedSegs;
	while (seg) {
		if (seg->start <= addr && seg->start + seg->len > addr) break;
		seg = seg->next;
	}

	if (seg != 0) {
		if (seg->seg->handle_fault(seg, addr, (regs->err_code & 0x2) && (regs->eip < 0xE0000000)) != 0) seg = 0;
	}

	if (seg == 0) {
		WHERE; monitor_write("Unhandled Page Fault ");
		if (regs->err_code & 0x1) monitor_write("present ");
		if (regs->err_code & 0x2) monitor_write("write ");
		if (regs->err_code & 0x4) monitor_write("user ");
		if (regs->err_code & 0x8) monitor_write("rsvd ");
		if (regs->err_code & 0x10) monitor_write("instructionfetch ");
		monitor_write("cr2:"); monitor_writeHex(addr); monitor_write("\n");
		return 1;
	}
	return 0;
}

/*	Gets the corresponding page in a page directory for a given address.
	If make is set, the necessary page table can be created.
	Can return 0 if make is not set. */
struct page *pagedir_getPage(struct page_directory *pd, uint32_t address, int make) {
	address /= 0x1000;
	uint32_t table_idx = address / 1024;

	if (pd->tables[table_idx]) {
		return &pd->tables[table_idx]->pages[address %  1024];
	} else if (make) {
		pd->tables[table_idx] = kmalloc_page(pd->tablesPhysical + table_idx);
		if (table_idx >= 896)
			tasking_updateKernelPagetable(table_idx, pd->tables[table_idx], pd->tablesPhysical[table_idx]);
		memset((uint8_t*)pd->tables[table_idx], 0, 0x1000);
		pd->tablesPhysical[table_idx] |= 0x07;
		return &pd->tables[table_idx]->pages[address %  1024];
	} else {
		return 0;
	}
}

/*	Modifies a page structure so that it is mapped to a frame. */
void page_map(struct page *page, uint32_t frame, uint32_t user, uint32_t rw) {
	if (page != 0 && page->frame == 0 && page->present == 0) {
		page->present = 1;
		page->rw = (rw ? 1 : 0);
		page->user = (user ? 1 : 0);
		page->frame = frame;
	}
}

/*	Modifies a page structure so that it is no longer mapped to a frame. */
void page_unmap(struct page *page) {
	if (page != 0) {
		page->frame = 0;
		page->present = 0;
	}
}

/*	Same as above but also frees the frame. */
void page_unmapFree(struct page *page) {
	if (page != 0) {
		if (page->frame != 0) frame_free(page->frame);
		page->frame = 0;
		page->present = 0;
	}
}
