#ifndef DEF_TASK_H
#define DEF_TASK_H

#include <types.h>
#include <mem/paging.h>
#include "idt.h"

struct process {
	uint32_t pid, uid;
	struct process *parent;
	struct page_directory *pagedir;

	struct process *next;	//Forms a linked list
};

#define TS_RUNNING 0	
#define TS_SLEEPING 1	//Sleeping for a defined amount of time
#define TS_WAIKWAIT 2	//Waiting to be waked up by something precise (thread currently blocked)

typedef void (*thread_entry)(void*);

struct thread {
	struct process *process;
	uint32_t esp, ebp, eip;
	uint8_t state;
	uint32_t timeWait;
	void* kernelStack_addr;
	uint32_t kernelStack_size;

	struct thread *next;	//Forms a linked list
};

extern struct thread *current_thread;

void tasking_init(thread_entry whereToGo, void *data);
void tasking_switch();
uint32_t tasking_handleException(struct registers *regs);

void thread_new(struct process *proc, thread_entry entry_point, void *data);

#endif
