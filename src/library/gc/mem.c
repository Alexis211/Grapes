#include "gc/mem.h"

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

int heap_create(struct heap *heap, size_t start, size_t idxsize, size_t datasize, size_t maxdatasize);
void* heap_alloc(struct heap *heap, size_t sz);
void heap_free(struct heap *heap, void* ptr);

#define HEAP_MAGIC 0xBAD0BEEF
#define HEAP_MIN_SIZE 0x4000

/* *******************			GENERAL MEMORY FUNCTIONS		******************** */

static struct heap h;
static int heap_ok = 0;

void* malloc(size_t size) {
	if (heap_ok == 0) {
		if (heap_create(&h, 0x20000000, 0x00010000, 0x00100000, 0x4F000000)) return 0;
		heap_ok = 1;
	}
	return heap_alloc(&h, size);
}

void free(void* p) {
	if (heap_ok == 0 || (size_t)p < h.start_addr || (size_t)p > h.end_addr) return;
	heap_free(&h, p);
}

static int setheapseg (struct heap *heap, size_t start, size_t end, size_t prev_end) {	//returns nonzero on error
	if (heap == &h) {
		return proc_setheap(start, end);
	}	//otherwise, something else might be done.
	return -1;
}

/* *******************			HEAP HEADER    	   ****************** */

static void heapIdx_insert(struct heap *heap, struct heap_header *e) {
	if ((heap->idxused + sizeof(struct heap_header*) + (size_t)heap->idx) >= heap->start_addr) return;

	uint32_t iterator = 0, pos;
	while (iterator < heap->idxused && heap->idx[iterator]->size < e->size) {
		if (heap->idx[iterator] == e) return;
		iterator++;
	}
	if (iterator == heap->idxused) {
		heap->idx[heap->idxused++] = e;
	} else {
		pos = iterator;
		iterator = heap->idxused;
		while (iterator > pos) {
			heap->idx[iterator] = heap->idx[iterator - 1];
			iterator--;
		}
		heap->idxused++;
		heap->idx[pos] = e;
	}
}

static void heapIdx_remove(struct heap *heap, struct heap_header *e) {
	uint32_t iterator;
	for (iterator = 0; iterator < heap->idxused; iterator++) {
		if (heap->idx[iterator] == e) break;
	}
	if (iterator == heap->idxused) return;
	heap->idxused--;
	while (iterator < heap->idxused) {
		heap->idx[iterator] = heap->idx[iterator + 1];
		iterator++;
	}
}

/* ********************			HEAP CONTENTS		********************* */

int heap_create(struct heap *heap, size_t start, size_t idxsize, size_t datasize, size_t maxdatasize) {
	if (start & 0x0FFF) start = (start & 0xFFFFF000) + 0x1000;

	heap->start_addr = start + idxsize;
	heap->end_addr = start + idxsize + datasize;
	heap->max_end = start + idxsize + maxdatasize;

	if (setheapseg(heap, start, heap->end_addr, 0)) return -1;

	heap->idx = (struct heap_header**)start;	
	heap->idxused = 0;

	struct heap_header *hole = (struct heap_header*) heap->start_addr;
	hole->size = (heap->end_addr - heap->start_addr);
	hole->magic = HEAP_MAGIC;
	hole->is_hole = 1;

	struct heap_footer *hole_footer = (struct heap_footer*)(heap->end_addr - sizeof(struct heap_footer));
	hole_footer->header = hole;
	hole_footer->magic = HEAP_MAGIC;

	heapIdx_insert(heap, hole);
	return 0;
}

static uint32_t heap_expand(struct heap *heap, size_t quantity) {
	if (quantity & 0x0FFF) {
		quantity = (quantity & 0xFFFFF000) + 0x1000;
	}

	if (heap->end_addr + quantity > heap->max_end) return 0;

	size_t newEnd = heap->end_addr + quantity;

	if (setheapseg(heap, (size_t)heap->idx, newEnd, heap->end_addr)) return 0;	//failed to bigger segment

	struct heap_footer *last_footer = (struct heap_footer*)(heap->end_addr - sizeof(struct heap_footer));
	struct heap_header *last_header = last_footer->header;
	if (last_header->is_hole) {
		heapIdx_remove(heap, last_header);
		last_header->size += quantity;

		last_footer = (struct heap_footer*)(newEnd - sizeof(struct heap_footer));
		last_footer->magic = HEAP_MAGIC;
		last_footer->header = last_header;

		heapIdx_insert(heap, last_header);
	} else {
		last_header = (struct heap_header*)heap->end_addr;
		last_footer = (struct heap_footer*)(newEnd - sizeof(struct heap_footer));

		last_header->is_hole = 1;
		last_header->magic = HEAP_MAGIC;
		last_header->size = quantity;

		last_footer->magic = HEAP_MAGIC;
		last_footer->header = last_header;

		heapIdx_insert(heap, last_header);
	}

	heap->end_addr = newEnd;

	return 1;
}

static void heap_contract(struct heap *heap) {
	struct heap_footer *last_footer = (struct heap_footer*)(heap->end_addr - sizeof(struct heap_footer));
	struct heap_header *last_header = last_footer->header;

	if (last_header->is_hole == 0) return;

	size_t quantity = 0;
	while ((heap->end_addr - heap->start_addr) - quantity > HEAP_MIN_SIZE &&
			(last_header->size - quantity) > 0x1000)
		quantity += 0x1000;
	if (quantity == 0) return;

	size_t newEnd = heap->end_addr - quantity;

	if (setheapseg(heap, (size_t)heap->idx, newEnd, heap->end_addr)) return; //error ocurred

	heapIdx_remove(heap, last_header);
	last_header->size -= quantity;
	last_footer = (struct heap_footer*)((size_t)last_footer - quantity);
	last_footer->magic = HEAP_MAGIC;
	last_footer->header = last_header;
	heapIdx_insert(heap, last_header);

	heap->end_addr = newEnd;
}

void* heap_alloc(struct heap *heap, size_t sz) {
	size_t newsize = sz + sizeof(struct heap_header) + sizeof(struct heap_footer);
	uint32_t iterator = 0;

	while (iterator < heap->idxused) {
		if (heap->idx[iterator]->size >= newsize) break;
		iterator++;
	}

	if (iterator == heap->idxused) {	//No hole is big enough
		if (heap_expand(heap, (sz & 0xFFFFF000) + 0x1000) == 0) return 0;	//FAILED
		return heap_alloc(heap, sz);
	}

	struct heap_header *loc = heap->idx[iterator];
	struct heap_footer *footer = (struct heap_footer*)((size_t)loc + loc->size - sizeof(struct heap_footer));
	loc->is_hole = 0;
	heapIdx_remove(heap, loc);
	
	//If we have enough space to create a USEFUL new hole next to the allocated block, do it.
	//If we do not, we might return a block that is a few bytes bigger than neede.
	if (loc->size > (newsize + sizeof(struct heap_header) + sizeof(struct heap_footer))) {
		loc->size = newsize;

		//Write footer for block we return
		struct heap_footer *newfooter = (struct heap_footer*)((size_t)loc + newsize - sizeof(struct heap_footer));
		newfooter->header = loc;
		newfooter->magic = HEAP_MAGIC;

		//Write header for new hole we create
		struct heap_header *nextloc = (struct heap_header*)((size_t)loc + newsize);
		nextloc->is_hole = 1;
		nextloc->magic = HEAP_MAGIC;
		nextloc->size = ((size_t)footer - (size_t)nextloc + sizeof(struct heap_footer));
		footer->header = nextloc;	//Update footer
		footer->magic = HEAP_MAGIC;

		heapIdx_insert(heap, nextloc);
	}

	return (void*)((size_t)loc + sizeof(struct heap_header));
}

void heap_free(struct heap *heap, void* ptr) {
	if (ptr == 0) return;
	if ((size_t)ptr < heap->start_addr || (size_t)ptr > heap->end_addr) return;

	struct heap_header *header = (struct heap_header*)((size_t)ptr - sizeof(struct heap_header));
	struct heap_footer *footer = (struct heap_footer*)((size_t)header + header->size - sizeof(struct heap_footer));
	if (header->magic != HEAP_MAGIC || footer->magic != HEAP_MAGIC) return;

	//Unify left
	struct heap_footer *prev_footer = (struct heap_footer*)((size_t)header - sizeof(struct heap_footer));
	if (prev_footer->magic == HEAP_MAGIC && prev_footer->header->is_hole) {
		header = prev_footer->header;
		heapIdx_remove(heap, header);

		footer->header = header;
		header->size = ((size_t)footer - (size_t)header + sizeof(struct heap_footer));
	}

	//Unify right
	struct heap_header *next_header = (struct heap_header*)((size_t)footer + sizeof(struct heap_footer));
	if (next_header->magic == HEAP_MAGIC && next_header->is_hole) {
		heapIdx_remove(heap, next_header);
		footer = (struct heap_footer*)((size_t)footer + next_header->size);

		footer->header = header;
		header->size = ((size_t)footer - (size_t)header + sizeof(struct heap_footer));
	}

	header->is_hole = 1;

	heapIdx_insert(heap, header);

	if ((size_t)footer == (heap->end_addr - sizeof(struct heap_footer)) &&
			header->size >= 0x2000 && (heap->end_addr - heap->start_addr > HEAP_MIN_SIZE)) {
		heap_contract(heap);
	}	
}
