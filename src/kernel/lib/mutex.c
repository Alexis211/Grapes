#include "mutex.h"
#include <task/task.h>

/*	Internal use only. This function is atomic, meaning it cannot be interrupted by a system task switch. */
static uint32_t atomic_exchange(uint32_t* ptr, uint32_t newval) {
	uint32_t r;
	asm volatile("xchg (%%ecx), %%eax" : "=a"(r) : "c"(ptr), "a"(newval));
	return r;
}

void mutex_lock(uint32_t* mutex) {
	while (atomic_exchange(mutex, MUTEX_LOCKED) == MUTEX_LOCKED) {
		thread_sleep(1);
	}
}

int mutex_lockE(uint32_t* mutex) {
	if (atomic_exchange(mutex, MUTEX_LOCKED) == MUTEX_LOCKED) return 0;
	return 1;
}

void mutex_unlock(uint32_t* mutex) {
	*mutex = MUTEX_UNLOCKED;
}
