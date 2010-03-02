#ifndef DEF_SHM_H
#define DEF_SHM_H

#include <task/task.h>

struct shmseg {
	size_t len;
	uint32_t *frames;
	struct process* owner;
};

//Shared memory segment stuff
struct segment* shmseg_make(size_t len, struct process* owner);
struct segment_map* shmseg_map(struct segment* seg, struct page_directory *pagedir, size_t offset);
void shmseg_unmap(struct segment_map*);
void shmseg_delete(struct segment *seg);
int shmseg_handleFault(struct segment_map *map, size_t addr, int write);

//Shared memory syscalls
void shm_create(size_t offset, size_t len);
void shm_delete(size_t offset);

#endif

