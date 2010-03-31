#ifndef DEF_MUTEX_H
#define DEF_MUTEX_H

#include <gc/syscall.h>

#define MUTEX_LOCKED 1
#define MUTEX_UNLOCKED 0

//A mutex is just an uint32_t

void mutex_lock(uint32_t* mutex);	//wait for mutex to be free
int mutex_lockE(uint32_t* mutex);	//lock mutex only if free, returns !0 if locked, 0 if was busy
void mutex_unlock(uint32_t* mutex);

#endif
