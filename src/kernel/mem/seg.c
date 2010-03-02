#include "seg.h"
#include "mem.h"

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

struct segment_map* simpleseg_map(struct segment* seg, struct page_directory* pagedir, size_t offset) {
	struct segment_map *sm = kmalloc(sizeof(struct segment_map));
	sm->start = ((struct simpleseg*)(seg->seg_data))->start;
	sm->len = ((struct simpleseg*)(seg->seg_data))->len;
	return sm;
}

void simpleseg_unmap(struct segment_map* sm) {
	size_t i;
	for (i = sm->start; i < sm->start + sm->len; i += 0x1000) {
		page_unmapFree(pagedir_getPage(sm->pagedir, i, 0));
	}
}

int simpleseg_handleFault(struct segment_map* sm, size_t addr, int write) {
	struct simpleseg *ss = sm->seg->seg_data;
	if (write && !ss->writable) return 1;
	addr &= 0xFFFFF000;
	struct page *p = pagedir_getPage(sm->pagedir, addr, 1);
	if (p->frame != 0) return 1;
	page_map(p, frame_alloc(), 1, ss->writable);
	return 0;
}

void simpleseg_delete(struct segment* seg) {
}
