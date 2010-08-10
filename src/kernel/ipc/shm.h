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
		//find a shared memory segment in current address space by its offset
struct segment_map* shmseg_getByOff(struct process* pr, size_t offset);	

//Shared memory syscalls
int shm_create(size_t offset, size_t len);
int shm_delete(size_t offset);

#endif

