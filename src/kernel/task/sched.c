#include "sched.h"
#include <core/sys.h>
#include <mem/mem.h>

// Lower priority numbers have high priority. Priorities must start at 0.
#define PRIORITIES 4		// we have 4 priority levels
#define PRIORITY(t) (t->process->privilege)		//get priority for a thread

extern struct thread *idle_thread;

static struct thread *queue[PRIORITIES] = {0}, *last[PRIORITIES] = {0};

/*	For internal use only. Enqueues specified thread in specified priority queue. */
static void sched_enqueueIn(struct thread *t, int qid) {
	t->queue_next = 0;
	if (queue[qid] == 0) {
		queue[qid] = last[qid] = t;
	} else {
		last[qid]->queue_next = t;
		last[qid] = t;
	}
}

/*	For internal use only. Pops a thread from specified queue, if available. */
static struct thread *sched_dequeueFrom(int qid) {
	if (queue[qid] == 0) return 0;
	struct thread *it = queue[qid];
	ASSERT((it->queue_next == 0 && it == last[qid]) || it != last[qid]);
	queue[qid] = it->queue_next;
	if (queue[qid] == 0) last[qid] = 0;
	return it;
}

/*	Used by task.c. Enqueus a thread in the corresponding priority queue. */
void sched_enqueue(struct thread *t) {
	if (t == idle_thread) return;
	sched_enqueueIn(t, PRIORITY(t));
}

/*	Used by task.c. Pops a thread from the lowest priority non-empty queue. */
struct thread *sched_dequeue() {
	struct thread *it = 0;
	int i;
	for (i = 0; i < PRIORITIES; i++) {
		it = sched_dequeueFrom(i);
		if (it != 0) break;
	}
	if (it == 0) return idle_thread;
	return it;
}
