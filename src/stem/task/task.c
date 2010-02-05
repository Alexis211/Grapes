#include "task.h"
#include <core/sys.h>
#include <core/monitor.h>
#include <mem/mem.h>
#include "timer.h"

#define KSTACKSIZE 0x8000

//From task_.asm
extern uint32_t read_eip();
extern void task_idle(void*);

static uint32_t thread_runnable(struct thread *th);

static uint32_t nextpid = 1;

struct process *processes = 0, *kernel_process;
struct thread *threads = 0, *current_thread = 0, *idle_thread;

uint32_t tasking_tmpStack[0x4000];

void tasking_init(thread_entry whereToGo, void *data) {
	cli();
	kernel_process = kmalloc(sizeof(struct process));	//This process must be hidden to users
	kernel_process->pid = kernel_process->uid = kernel_process->threads = 0;
	kernel_process->privilege = PL_KERNEL;
	kernel_process->parent = kernel_process;
	kernel_process->pagedir = kernel_pagedir;
	kernel_process->next = 0;
	current_thread = 0;
	thread_new(kernel_process, task_idle, 0);
	thread_new(kernel_process, whereToGo, data);
	sti();
	monitor_write("Tasking starting\n");
	tasking_switch();
}

static struct thread *thread_next() {		//NOT OPTIMAL : will allocate time slices to idle thread even if busy
	if (current_thread == 0) current_thread = threads;
	struct thread *ret = current_thread;
	while (1) {
		ret = ret->next;
		if (ret == 0) ret = threads;
		if (thread_runnable(ret)) {
		   return ret;
		}
	}
}

void tasking_switch() {
	if (threads == 0) return;	//no tasking yet
	cli();

	uint32_t esp, ebp, eip;

	asm volatile("mov %%esp, %0" : "=r"(esp));
	asm volatile("mov %%ebp, %0" : "=r"(ebp));
	eip = read_eip();

	if (eip == 0x12345) {
		return;
	}

	if (current_thread != 0) {
		current_thread->esp = esp;
		current_thread->ebp = ebp;
		current_thread->eip = eip;
	}

	current_thread = thread_next();

	asm volatile("			\
			mov %0, %%ebp;	\
			mov %1, %%esp;	\
			mov %2, %%ecx;	\
			mov $0x12345, %%eax;	\
			jmp *%%ecx;"
		: : "r"(current_thread->ebp), "r"(current_thread->esp), "r"(current_thread->eip));
}

void tasking_updateKernelPagetable(uint32_t idx, struct page_table *table, uint32_t tablephysical) {
	struct process* it = processes;
	while (it != 0) {
		it->pagedir->tables[idx] = table;
		it->pagedir->tablesPhysical[idx] = tablephysical;
		it = it->next;
	}
}

uint32_t tasking_handleException(struct registers *regs) {
	if (threads == 0) return 0;	//No tasking yet
	return 0;
}

static uint32_t thread_runnable(struct thread *t) {
	if (t->state == TS_RUNNING) return 1;
	if (t->state == TS_SLEEPING && timer_time() >= t->timeWait) return 1;
	return 0;
}

static void thread_run(struct thread *thread, thread_entry entry_point, void *data) {
	pagedir_switch(thread->process->pagedir);	//TODO : take into account privilege level
	asm volatile("sti");
	entry_point(data);
	asm volatile("int $64");
}

struct thread *thread_new(struct process *proc, thread_entry entry_point, void *data) {
	struct thread *t = kmalloc(sizeof(struct thread));
	t->process = proc;
	proc->threads++;
	t->kernelStack_addr = kmalloc(KSTACKSIZE);
	t->kernelStack_size = KSTACKSIZE;

	uint32_t *stack = (uint32_t*)((size_t)t->kernelStack_addr + t->kernelStack_size);

	//Pass parameters
	stack--; *stack = (uint32_t)data;
	stack--; *stack = (uint32_t)entry_point;	
	stack--; *stack = (uint32_t)t;
	stack--; *stack = 0;
	t->esp = (uint32_t)stack;
	t->ebp = t->esp + 8;
	t->eip = (uint32_t)thread_run;

	t->state = TS_RUNNING;

	if (threads == 0) {
		threads = t;
	} else {
		struct thread *i = threads;
		while (i->next != 0) i = i->next;
		i->next = t;
	}
	return t;
}

struct process *process_new(struct process* parent, uint32_t uid, uint32_t privilege) {
	struct process* p = kmalloc(sizeof(struct process));
	p->pid = (nextpid++);
	p->uid = uid;
	p->threads = 0;
	p->privilege = privilege;
	p->parent = parent;
	p->pagedir = pagedir_new();
	p->next = processes;
	processes = p;
	return p;
}
