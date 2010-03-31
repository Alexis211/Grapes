#ifndef DEF_SHM_H
#define DEF_SHM_H

#include <gc/syscall.h>

/*
 * This file contains headers for the shared segment mapping manager.
 */

void* shm_alloc(size_t size);	//allocates a spot in shared memory space
void shm_free(void* p);			//frees a spot

void* shm_allocNew(size_t size);	//calls shm_alloc, and maps a new segment there
void shm_freeDel(void* p);			//unmaps segment and calls shm_free

#endif
