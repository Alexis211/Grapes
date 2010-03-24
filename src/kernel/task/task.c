#include "task.h"
#include <core/sys.h>
#include <core/monitor.h>
#include <mem/mem.h>
#include <mem/seg.h>
#include <mem/gdt.h>
#include <ipc/object.h>
#include "timer.h"

#define KSTACKSIZE 0x8000

//Static routines for handling threads exiting and all cleanup
static void thread_exit_stackJmp(uint32_t reason);
static void thread_exit2(uint32_t reason);
static void thread_delete(struct thread *th);
static void process_delete(struct process *pr);

//From task_.asm
extern uint32_t read_eip();
extern void task_idle(void*);

static uint32_t thread_runnable(struct thread *th);

static uint32_t nextpid = 1;

struct process *processes = 0, *kernel_process;
struct thread *threads = 0, *current_thread = 0, *idle_thread;

uint32_t tasking_tmpStack[0x4000];

void tasking_init() {
	cli();
	kernel_process = kmalloc(sizeof(struct process));	//This process must be hidden to users
	kernel_process->pid = kernel_process->uid = kernel_process->threads = 0;
	kernel_process->privilege = PL_KERNEL;
	kernel_process->parent = kernel_process;
	kernel_process->pagedir = kernel_pagedir;
	kernel_process->next = 0;
	current_thread = 0;
	idle_thread = thread_new(kernel_process, task_idle, 0);
	threads = 0;	//Do not include idle thread in threads
	sti();
	monitor_write("[Tasking] ");
}

static struct thread *thread_next() {
	if (current_thread == 0 || current_thread == idle_thread) current_thread = threads;
	struct thread *ret = current_thread;
	while (1) {
		ret = ret->next;
		if (ret == 0) ret = threads;
		if (thread_runnable(ret)) {
		   return ret;
		}
		if (ret == current_thread) return idle_thread;
	}
}

void tasking_switch() {
	if (threads == 0) PANIC("No more threads to run !");
	asm volatile("cli");

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

	gdt_setKernelStack(((uint32_t)current_thread->kernelStack_addr) + current_thread->kernelStack_size);

	asm volatile("			\
			mov %0, %%ebp;	\
			mov %1, %%esp;	\
			mov %2, %%ecx;	\
			mov $0x12345, %%eax;	\
			jmp *%%ecx;"
		: : "r"(current_thread->ebp), "r"(current_thread->esp), "r"(current_thread->eip));
}

void tasking_updateKernelPagetable(uint32_t idx, struct page_table *table, uint32_t tablephysical) {
	if (idx < 896) return;
	struct process* it = processes;
	while (it != 0) {
		it->pagedir->tables[idx] = table;
		it->pagedir->tablesPhysical[idx] = tablephysical;
		it = it->next;
	}
}

uint32_t tasking_handleException(struct registers *regs) {
	if (current_thread == 0) return 0;	//No tasking yet
	NL; WHERE; monitor_write("Unhandled exception : ");
	char *exception_messages[] = {"Division By Zero","Debug","Non Maskable Interrupt","Breakpoint",
    "Into Detected Overflow","Out of Bounds","Invalid Opcode","No Coprocessor", "Double Fault",
    "Coprocessor Segment Overrun","Bad TSS","Segment Not Present","Stack Fault","General Protection Fault",
    "Page Fault","Unknown Interrupt","Coprocessor Fault","Alignment Check","Machine Check"};
	monitor_write(exception_messages[regs->int_no]);
	monitor_write(" at "); monitor_writeHex(regs->eip);
	if (regs->int_no == 14) {
		monitor_write("\n>>> Process exiting.\n");
		thread_exit_stackJmp(EX_PR_EXCEPTION);
	} else {
		monitor_write("\n>>> Thread exiting.\n");
		thread_exit_stackJmp(EX_TH_EXCEPTION);
	}
	PANIC("This should never have happened. Please report this.");
	return 0;
}

void thread_sleep(uint32_t msecs) {
	if (current_thread == 0) return;
	current_thread->state = TS_SLEEPING;
	current_thread->timeWait = timer_time() + msecs;
	tasking_switch();
}

void thread_goInactive() {
	current_thread->state = TS_WAKEWAIT;
	tasking_switch();
}

void thread_wakeUp(struct thread* t) {
	if (t->state == TS_WAKEWAIT) t->state = TS_RUNNING;
}

int proc_priv() {
	if (current_thread == 0) return PL_UNKNOWN;
	return current_thread->process->privilege;
}

void thread_exit2(uint32_t reason) {		//See EX_TH_* defines in task.h
	/*
	 * if reason == EX_TH_NORMAL, it is just one thread exiting because it has to
	 * if reason == EX_TH_EXCEPTION, it is just one thread exiting because of an exception
	 * if reason is none of the two cases above, it is the whole process exiting (with error code = reason)
	 */
	struct thread *th = current_thread;
	struct process *pr = th->process;
	if ((reason == EX_TH_NORMAL || reason == EX_TH_EXCEPTION) && pr->threads > 1) {
		thread_delete(th);
	} else {
		process_delete(pr);
	}
	sti();
	tasking_switch();
}

void thread_exit_stackJmp(uint32_t reason) {
	uint32_t *stack;
	cli();
	stack = tasking_tmpStack + 0x4000;
	stack--; *stack = reason;
	stack--; *stack = 0;
	asm volatile("			\
			mov %0, %%esp;	\
			mov %1, %%ebp;	\
			mov %2, %%ecx;	\
			mov %3, %%cr3;	\
			jmp *%%ecx;" : :
			"r"(stack), "r"(stack), "r"(thread_exit2), "r"(kernel_pagedir->physicalAddr));
}

void thread_exit() {
	thread_exit_stackJmp(EX_TH_NORMAL);
}

void process_exit(uint32_t retval) {
	if (retval == EX_TH_NORMAL || retval == EX_TH_EXCEPTION) retval = EX_PR_EXCEPTION;
	thread_exit_stackJmp(retval);
}

static uint32_t thread_runnable(struct thread *t) {
	if (t->state == TS_RUNNING) return 1;
	if (t->state == TS_SLEEPING && timer_time() >= t->timeWait) {
		t->state = TS_RUNNING;
		return 1;
	}
	return 0;
}

static void thread_run(struct thread *thread, thread_entry entry_point, void *data) {
	pagedir_switch(thread->process->pagedir);
	if (thread->process->privilege >= PL_SERVICE) {	//User mode !
		uint32_t *stack = (uint32_t*)(thread->userStack_seg->start + thread->userStack_seg->len);

		stack--; *stack = (uint32_t)data;
		stack--; *stack = 0;
		size_t esp = (size_t)stack, eip = (size_t)entry_point;
		//Setup a false structure for returning from an interrupt :
		//value for esp is in ebx, for eip is in ecx
		//- update data segments to 0x23 = user data segment with RPL=3
		//- push value for ss : 0x23 (user data seg rpl3)
		//- push value for esp
		//- push flags
		//- update flags, set IF = 1 (interrupts flag)
		//- push value for cs : 0x1B = user code segment with RPL=3
		//- push eip
		//- return from fake interrupt
		asm volatile("				\
				mov $0x23, %%ax;	\
				mov %%ax, %%ds;		\
				mov %%ax, %%es;		\
				mov %%ax, %%fs;		\
				mov %%ax, %%gs;		\
									\
				pushl $0x23;		\
				pushl %%ebx;		\
				pushf;				\
				pop %%eax;			\
				or $0x200, %%eax;	\
				push %%eax;			\
				pushl $0x1B;		\
				push %%ecx;			\
				iret;				\
				" : : "b"(esp), "c"(eip));
	} else {
		asm volatile("sti");
		entry_point(data);
	}
	thread_exit(0);
}

struct thread *thread_new(struct process *proc, thread_entry entry_point, void *data) {
	struct thread *t = kmalloc(sizeof(struct thread));
	t->process = proc;
	proc->threads++;

	if (proc->privilege >= PL_SERVICE) {	//We are running in user mode
		proc->stacksBottom -= USER_STACK_SIZE;
		t->userStack_seg = seg_map(simpleseg_make(proc->stacksBottom, USER_STACK_SIZE, 1), proc->pagedir, 0);
	}

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
	p->stacksBottom = 0xDF000000;

	p->next_objdesc = 0;
	p->objects = 0;
	obj_createP(p);	//create process' root object and add descriptor 0 to it

	processes = p;
	return p;
}

static void thread_delete(struct thread *th) {
	kfree(th->kernelStack_addr);
	th->process->threads--;
	if (threads == th) {
		threads = th->next;
	} else {
		struct thread *it = threads;
		while (it->next != th && it->next != 0) it = it->next;
		if (it->next == th) it->next = th->next;
	}
	kfree(th);
}

static void process_delete(struct process *pr) {
	struct thread *it;
	while (threads != 0 && threads->process == pr) thread_delete(threads);
	it = threads;
	while (it != 0) {
		while (it->next->process == pr) thread_delete(it->next);
		it = it->next;
	}
	obj_closeall(pr);
	pagedir_delete(pr->pagedir);
	if (processes == pr) {
		processes = pr->next;
	} else {
		struct process *it = processes;
		while (it != 0 && it->next != pr) it = it->next;
		if (it != 0 && it->next == pr) it->next = pr->next;
	}
	kfree(pr);
}
