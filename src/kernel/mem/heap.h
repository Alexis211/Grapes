#ifndef DEF_HEAP_H
#define DEF_HEAP_H

/*	The heap is the data structure that permits allocating and freeing memory easily.
	The functions in this file are only used by mem.c, which provides kmalloc and kfree.
	The heap algorithm used is the one described here : 
	http://www.jamesmolloy.co.uk/tutorial_html/7.-The%20Heap.html */

#include "types.h"

struct heap_header {
	uint32_t magic;
	uint32_t is_hole;
	size_t size;
};

struct heap_footer {
	uint32_t magic;
	struct heap_header *header;
};

struct heap {
	struct heap_header **idx;
	uint32_t idxused;
	size_t start_addr, end_addr, max_end;
};

void heap_create(struct heap *heap, size_t start, size_t idxsize, size_t datasize, size_t maxdatasize);
void* heap_alloc(struct heap *heap, size_t sz);
void heap_free(struct heap *heap, void* ptr);

#endif
