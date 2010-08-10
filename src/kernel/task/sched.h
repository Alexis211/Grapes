#ifndef DEF_SCHED_H
#define DEF_SCHED_H

#include "task.h"

void sched_enqueue(struct thread *t);
struct thread *sched_dequeue();

#endif
