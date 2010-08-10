#include "mem.h"
#include <core/sys.h>
#include <core/monitor.h>
#include "paging.h"
#include "heap.h"

#define FREEPAGESTOKEEP 5

#define KHEAP_IDXSIZE 0x1000
#define KHEAP_INITSIZE 0x8000
#define KHEAP_MAXSIZE 0x08000000

size_t mem_placementAddr;
static uint32_t kheap_working = 0;


// ******************************
// 									PAGE ALLOCATION
// 															****************************
static struct freepage {
   size_t virt, phys;
} freepages[FREEPAGESTOKEEP];
uint32_t freepagecount = 0;

/*	For internal use only. Populates the cache of pages that can be given to requesters. */
static void get_free_pages() {
	static uint32_t locked = 0;
	uint32_t i;
	if (locked) return;
	locked = 1;
	while (freepagecount < FREEPAGESTOKEEP) {
		if (kheap_working) {
			for (i = 0xFFFFF000; i >= 0xF0000000; i -= 0x1000) {
				if (pagedir_getPage(kernel_pagedir, i, 1)->frame == 0) break;
			}
			freepages[freepagecount].virt = i;
			freepages[freepagecount].phys = frame_alloc() * 0x1000;
			page_map(pagedir_getPage(kernel_pagedir, i, 0), freepages[freepagecount].phys / 0x1000, 0, 0);
			freepagecount++;
		} else {
			if (mem_placementAddr & 0xFFFFF000) {
				mem_placementAddr &= 0xFFFFF000;
				mem_placementAddr += 0x1000;
			}
			freepages[freepagecount].virt = (size_t)kmalloc(0x1000);
			freepages[freepagecount].phys = freepages[freepagecount].virt - 0xE0000000;
			freepagecount++;
		}
	}
	locked = 0;
}

/*	Gives one page from the cache to someone requesting it. */
void* kmalloc_page(size_t *phys) {
	cli();
	get_free_pages();
	freepagecount--;
	*phys = freepages[freepagecount].phys;
	size_t tmp = freepages[freepagecount].virt;
	sti();
	return (void*)tmp;
}

void kfree_page(void* ptr) {
	size_t addr = (size_t)ptr;
	if (kheap_working) {		//With this we can know if paging works
		page_unmapFree(pagedir_getPage(kernel_pagedir, addr, 0));
	}
}

//***********************************
//										NORMAL MEMORY ALLOCATION
//																	*************************

static struct heap kheap;

/*	Called on kernel start. Creates the kernel heap. */
void kheap_init() {
	heap_create(&kheap, (mem_placementAddr & 0xFFFFF000) + 0x1000, KHEAP_IDXSIZE, KHEAP_INITSIZE, KHEAP_MAXSIZE);
	kheap_working = 1;
	monitor_write("[KHeap] ");
}

/*	Allocates on the heap if possible. If not possible, allocates just after the kernel code. */
void* kmalloc(size_t size) {
	if (kheap_working) {
		return heap_alloc(&kheap, size);
	} else {
		size_t tmp = mem_placementAddr;
		mem_placementAddr += size;
		return (void*)tmp;
	}
}

void kfree(void* ptr) {
	if (kheap_working) {
		heap_free(&kheap, ptr);
	}
}
