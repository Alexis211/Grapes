#include "seg.h"
#include "mem.h"
#include <core/sys.h>

/*	Call this function when mapping a segment to a page directory.
	Calls the appropriate map method and updates the segment's and pagedir's information. */
struct segment_map *seg_map(struct segment* seg, struct page_directory *pagedir, size_t offset) {
	struct segment_map *sm = seg->map(seg, pagedir, offset);
	if (sm == 0) return 0;
	seg->mappings++;
	sm->seg = seg;
	sm->pagedir = pagedir;
	sm->next = pagedir->mappedSegs;
	pagedir->mappedSegs = sm;
	return sm;
}

/*	Call this function when unmapping a segment from a page directory.
	The segment will automatically be deleted if it is not mapped.
	Calls the appropriate unmap method and updates the segment's and pagedir's information. */
void seg_unmap(struct segment_map *map) {
	map->seg->unmap(map);
	if (map->pagedir->mappedSegs == map) {
		map->pagedir->mappedSegs = map->pagedir->mappedSegs->next;
	} else {
		struct segment_map *m = map->pagedir->mappedSegs;
		while (m->next != 0 && m->next != map) m = m->next;
		if (m->next == map) m->next = map->next;
	}
	map->seg->mappings--;
	if (map->seg->mappings == 0) {
		map->seg->delete(map->seg);
		kfree(map->seg->seg_data);
		kfree(map->seg);
	}
	kfree (map);
}

// ************************************   SIMPLESEG stuff *************
 
static struct segment_map* simpleseg_map(struct segment* seg, struct page_directory* pagedir, size_t offset);
static void simpleseg_unmap(struct segment_map*);
static void simpleseg_delete(struct segment *seg);
static int simpleseg_handleFault(struct segment_map* map, size_t addr, int write);

/*	Call this when creating a simpleseg.
	Creates the simpleseg structure and the segment structure and fills them up. */
struct segment* simpleseg_make(size_t start, size_t len, int writable) {
	struct simpleseg *ss = kmalloc(sizeof(struct simpleseg));
	struct segment *se = kmalloc(sizeof(struct segment));
	se->seg_data = ss;
	se->mappings = 0;
	se->map = simpleseg_map;
	se->unmap = simpleseg_unmap;
	se->delete = simpleseg_delete;
	se->handle_fault = simpleseg_handleFault;
	ss->writable = writable; ss->start = start; ss->len = len;
	return se;
}

/*	For internal use only. Called when a simpleseg is mapped to a pagedirectory. */
struct segment_map* simpleseg_map(struct segment* seg, struct page_directory* pagedir, size_t offset) {
	struct segment_map *sm = kmalloc(sizeof(struct segment_map));
	sm->start = ((struct simpleseg*)(seg->seg_data))->start;
	sm->len = ((struct simpleseg*)(seg->seg_data))->len;
	return sm;
}

/*	For internal use only. Called when a simpleseg is unmapped.
	Frees all the allocated pages. */
void simpleseg_unmap(struct segment_map* sm) {
	size_t i;
	for (i = sm->start; i < sm->start + sm->len; i += 0x1000) {
		page_unmapFree(pagedir_getPage(sm->pagedir, i, 0));
	}
}

/*	For internal use only. Handles a page fault. Can allocate and map a frame if necessary. */
int simpleseg_handleFault(struct segment_map* sm, size_t addr, int write) {
	struct simpleseg *ss = sm->seg->seg_data;
	if (write && !ss->writable) return 1;
	addr &= 0xFFFFF000;
	struct page *p = pagedir_getPage(sm->pagedir, addr, 1);
	if (p->frame != 0) return 1;
	page_map(p, frame_alloc(), 1, ss->writable);
	return 0;
}

/*	For internal use only. Called when the simpleseg is deleted. Does nothing. */
void simpleseg_delete(struct segment* seg) {
}

/*	Call this to resize a simpleseg. Ajusts the size and frees pages if the new size is smaller.*/
int simpleseg_resize(struct segment_map *map, size_t len) {
	size_t i;

	if (map == 0) return -1;
	if (map->seg->delete != simpleseg_delete) return -2;	//check segment is a simpleseg

	struct simpleseg *s = (struct simpleseg*)map->seg->seg_data;
	if (len & 0xFFF) len = (len & 0xFFFFF000) + 0x1000;
	if (len < map->len) {
		for (i = map->start + len; i < map->start + map->len; i += 0x1000) {
			page_unmapFree(pagedir_getPage(map->pagedir, i, 0));
		}
		map->len = len;
		s->len = len;
	} else if (len > map->len) {
		map->len = len;
		s->len = len;
	}
	return 0;
}
