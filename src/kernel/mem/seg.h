#ifndef DEF_SEG_H
#define DEF_SEG_H

#include "paging.h"

struct segment_map;
struct segment {
	void* seg_data;
	int mappings;

	struct segment_map* (*map)(struct segment* seg, struct page_directory* pagedir, size_t offset);
	void (*unmap)(struct segment_map*);
	void (*delete)(struct segment* seg);
	int (*handle_fault)(struct segment_map* map, size_t addr, int write);	//0 if ok, 1 if segfault
};

struct segment_map {
	struct segment* seg;
	struct page_directory* pagedir;
	size_t start, len;
	struct segment_map *next;
};

//parameter offset in seg_map needs not be used
struct segment_map *seg_map(struct segment* seg, struct page_directory* pagedir, size_t offset);
void seg_unmap(struct segment_map* map);

/// *************************************     SIMPLESEG stuff *****************

struct simpleseg {
	int writable;
	size_t start, len;
};

struct segment* simpleseg_make(size_t start, size_t len, int writable); 
struct segment_map* simpleseg_map(struct segment* seg, struct page_directory* pagedir, size_t offset);
void simpleseg_unmap(struct segment_map*);
void simpleseg_delete(struct segment *seg);
int simpleseg_handleFault(struct segment_map* map, size_t addr, int write);

#endif
