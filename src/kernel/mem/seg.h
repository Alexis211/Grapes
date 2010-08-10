#ifndef DEF_SEG_H
#define DEF_SEG_H

/*	Segments are pieces of usable memory in a process' address space.
	They have nothing to do with GDT segments, they are created over paging. */

#include "paging.h"

struct segment_map;
struct segment {
	void* seg_data;
	int mappings;

	// these 4 functions must not be used directly by anyone
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

//parameter offset in seg_map doesn't need to be used
struct segment_map *seg_map(struct segment* seg, struct page_directory* pagedir, size_t offset);
/*	When unmapping a segment, the segment is deleted if it is not mapped anywhere anymore. */
void seg_unmap(struct segment_map* map);

/// *************************************     SIMPLESEG stuff *****************

struct simpleseg {
	int writable;
	size_t start, len;
};

struct segment* simpleseg_make(size_t start, size_t len, int writable);
int simpleseg_resize(struct segment_map *map, size_t len);

#endif
