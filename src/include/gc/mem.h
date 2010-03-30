#ifndef DEF_HEAP_H
#define DEF_HEAP_H

#include "gc/syscall.h"

void* malloc(size_t size);
void free(void* p);

#endif
