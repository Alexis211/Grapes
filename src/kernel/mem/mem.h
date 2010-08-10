#ifndef DEF_MEM_H
#define DEF_MEM_H

/*	mem.c provides the kernel's allocation and freeing functions.
	kmalloc_page and kfree_page are used mostly for paging. */

#include <types.h>

void* kmalloc_page(size_t *phys);
void kfree_page(void* page);
void* kmalloc(size_t size);
void kfree(void* ptr);

void kheap_init();

extern size_t mem_placementAddr;
extern void end;	//Symbol defined by linker : end of kernel code

#endif

