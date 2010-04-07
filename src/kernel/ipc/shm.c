#include "shm.h"
#include <mem/mem.h>
#include <mem/seg.h>
#include <task/task.h>
#include <core/sys.h>

struct segment* shmseg_make(size_t len, struct process* owner) {
	struct shmseg *ss = kmalloc(sizeof(struct shmseg));
	struct segment *se = kmalloc(sizeof(struct segment));
	unsigned i;
	se->seg_data = ss;
	se->mappings = 0;
	se->map = shmseg_map;
	se->unmap = shmseg_unmap;
	se->delete = shmseg_delete;
	se->handle_fault = shmseg_handleFault;
	ss->len = len;
	ss->owner = owner;
	ss->frames = kmalloc((len / 0x1000) * sizeof(uint32_t));
	for (i = 0; i < (len / 0x1000); i++) ss->frames[i] = 0;
	return se;
}

struct segment_map *shmseg_map(struct segment *seg, struct page_directory *pagedir, size_t offset) {
	struct segment_map *sm = kmalloc(sizeof(struct segment_map));
	sm->start = offset;
	sm->len = ((struct shmseg*)(seg->seg_data))->len;
	return sm;
}

void shmseg_unmap(struct segment_map *sm) {
	size_t i;
	for (i = 0; i < sm->len; i += 0x1000) {
		struct page *page = pagedir_getPage(sm->pagedir, sm->start + i, 0);
		if (page != 0) page_unmap(page);
	}
}

int shmseg_handleFault(struct segment_map *sm, size_t addr, int write) {
	struct shmseg *ss = sm->seg->seg_data;
	addr &= 0xFFFFF000;
	struct page *p = pagedir_getPage(sm->pagedir, addr, 1);
	if (p->frame != 0) return 1;
	int frame_idx = (addr - sm->start) / 0x1000;
	if (ss->frames[frame_idx] == 0) {
		ss->frames[frame_idx] = frame_alloc();
	}
	page_map(p, ss->frames[frame_idx], 1, 1);
	return 0;
}

void shmseg_delete(struct segment *seg) {
	struct shmseg *ss = seg->seg_data;
	unsigned i;
	for (i = 0; i < (ss->len / 0x1000); i++) {
		if (ss->frames[i] != 0) frame_free(ss->frames[i]);
	}
	kfree(ss->frames);
}

struct segment_map* shmseg_getByOff(struct process* pr, size_t offset) {
	struct segment_map* m = pr->pagedir->mappedSegs;
	while (m != 0) {
		if (m->start == offset && m->seg->delete == shmseg_delete) return m;
		m = m->next;
	}
	return 0;
}

// **** **** SHM syscalls **** ****
int shm_create(size_t offset, size_t len) {
	if (offset >= 0xE0000000) return -1;
	if (len >= 0x10000000) return -1;
	if (offset+len >= 0xE0000000) return -1;
	seg_map(shmseg_make(len, current_thread->process), current_thread->process->pagedir, offset);
	return 0;
}

int shm_delete(size_t offset) {
	struct segment_map *s = shmseg_getByOff(current_thread->process, offset);
	if (s == 0) return -1;
	seg_unmap(s);
	return 0;
}
