#include "sched.h"
#include <core/sys.h>
#include <mem/mem.h>

static struct thread *queue = 0, *last = 0;

/*	Used by task.c. Enqueus a thread in the queue of threads waiting to run. */
void sched_enqueue(struct thread *t) {
	t->queue_next = 0;
	if (queue == 0) {
		queue = last = t;
	} else {
		last->queue_next = t;
		last = t;
	}
}

/*	Used by task.c. Pops a thread from the queue to be run. */
struct thread *sched_dequeue() {
	if (queue == 0) return 0;
	struct thread *it = queue;
	ASSERT((it->queue_next == 0 && it == last) || it != last);
	queue = it->queue_next;
	if (queue == 0) last = 0;
	return it;
}
