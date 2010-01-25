#ifndef DEF_MEM_H
#define DEF_MEM_H

#include "types.h"

void* kmalloc_page(size_t *phys);
void kfree_page(void* page);
void* kmalloc(size_t size);
void kfree(void* ptr);

void kheap_init();

extern size_t mem_placementAddr;
extern void end;	//Symbol defined by linker : end of kernel code

#endif

