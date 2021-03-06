#ifndef DEF_TASK_H
#define DEF_TASK_H

#include <types.h>
#include <mem/paging.h>
#include "idt.h"

#define TS_RUNNING 0	
#define TS_WAKEWAIT 2	//Waiting to be waked up by something precise (thread currently blocked)

#define PL_UNKNOWN 4
#define PL_USER 3
#define PL_SERVICE 2
#define PL_DRIVER 1
#define PL_KERNEL 0

#define EX_TH_NORMAL	0x10000		//ERROR code : just one thread exits, because it has to
#define EX_TH_EXCEPTION 0x10001		//ERROR code : just one thread exits, because of an unhandled exception
#define EX_PR_EXCEPTION 0x10002		//ERROR code : all process finishes, because of an unhandled exception

#define USER_STACK_SIZE 0x8000	//32k, but pages will be mapped one by one as used

typedef void (*thread_entry)(void*);

struct process {
	uint32_t pid, uid, privilege, thread_count;
	struct process *parent;
	struct page_directory *pagedir;
	size_t stacksBottom;

	struct obj_descriptor *objects;
	uint32_t next_objdesc;

	struct segment_map *heapseg;

	struct process *next;	//Forms a linked list
	struct thread *threads;
};

struct thread {
	struct process *process;
	uint32_t esp, ebp, eip;
	uint8_t state;
	uint32_t timeWait;
	void* kernelStack_addr;
	uint32_t kernelStack_size;
	struct segment_map *userStack_seg;

	struct thread *next, *queue_next;	//queue_next is used in sched.c
};

extern struct thread *current_thread;

void tasking_init();
void tasking_switch();
void tasking_updateKernelPagetable(uint32_t idx, struct page_table *table, uint32_t tablePhysical);
uint32_t tasking_handleException(struct registers *regs);

void thread_goInactive();	//Blocks the current thread. it is then waked up by another thread or a system event. 
void thread_wakeUp(struct thread *t);
int proc_priv();	//Returns current privilege level
struct thread * thread_new(struct process *proc, thread_entry entry_point, void *data);
struct process* process_new(struct process *parent, uint32_t uid, uint32_t privilege);

void thread_exit();	//syscall
void process_exit(uint32_t retval);	//syscall
int process_setheapseg(size_t start, size_t end);	//syscall

#endif
