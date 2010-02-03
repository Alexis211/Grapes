#ifndef DEF_PAGING_H
#define DEF_PAGING_H

#include <types.h>
#include <task/idt.h>

struct page {
	uint32_t present	: 1;	//Page mapped to a frame ?
	uint32_t rw			: 1;	//Page read/write ?
	uint32_t user		: 1;	//Page user readable ?
	uint32_t accessed	: 1;	//Was page accessed ?
	uint32_t dirty		: 1;	//Was page modified ?
	uint32_t unused		: 7;
	uint32_t frame		: 20;	//Frame address (physical address)
};

struct page_table {
	struct page pages[1024];
};

struct page_directory {
	struct page_table *tables[1024];	//Virtual addresses of page tables
	uint32_t *tablesPhysical;			//Pointer to the virtual address of the page directory (contain phys addr of pt)
	uint32_t physicalAddr;				//Physical address of info above
};

extern struct page_directory *kernel_pagedir;

uint32_t frame_alloc();
void frame_free(uint32_t id);

void paging_init(size_t totalRam);
void paging_cleanup();
void pagedir_switch(struct page_directory *pd);
struct page *pagedir_getPage(struct page_directory *pd, uint32_t address, int make);
void page_map(struct page *page, uint32_t frame, uint32_t user, uint32_t rw);
void page_unmap(struct page *page);
void page_unmapFree(struct page *page);

uint32_t paging_fault(struct registers *regs);	//returns a boolean

#endif
