#include "task.h"
#include "sched.h"
#include <core/sys.h>
#include <core/monitor.h>
#include <mem/mem.h>
#include <mem/seg.h>
#include <mem/gdt.h>
#include <ipc/object.h>
#include "timer.h"

#define KSTACKSIZE 0x8000

static struct object *manager_object = 0;

//Static routines for handling threads exiting and all cleanup
static void thread_exit_stackJmp(uint32_t reason);
static void thread_exit2(uint32_t reason);
static void thread_delete(struct thread *th);
static void process_delete(struct process *pr);

//From task_.asm
extern uint32_t read_eip();
extern void task_idle(void*);

static uint32_t nextpid = 1;
struct process *processes = 0, *kernel_process;
struct thread *current_thread = 0, *idle_thread = 0;

uint32_t tasking_tmpStack[KSTACKSIZE];

/*	Sets up tasking. Called by kmain on startup.
	Creates a kernel process and an IDLE thread in it. */
void tasking_init() {
	cli();
	kernel_process = kmalloc(sizeof(struct process));	//This process must be hidden to users
	kernel_process->pid = kernel_process->uid = kernel_process->thread_count = 0;
	kernel_process->privilege = PL_KERNEL;
	kernel_process->parent = kernel_process;
	kernel_process->pagedir = kernel_pagedir;
	kernel_process->next = 0;
	current_thread = 0;
	idle_thread = thread_new(kernel_process, task_idle, 0);
	kernel_process->threads = idle_thread;
	sti();
	monitor_write("[Tasking] ");
}

/*	Called by the paging functions when a page table is allocated in the kernel space (>0xE0000000).
	Updates the page directories of all the processes. */
void tasking_updateKernelPagetable(uint32_t idx, struct page_table *table, uint32_t tablephysical) {
	if (idx < 896) return;
	struct process* it = processes;
	while (it != 0) {
		it->pagedir->tables[idx] = table;
		it->pagedir->tablesPhysical[idx] = tablephysical;
		it = it->next;
	}
}

/*	Called when a timer IRQ fires. Does a context switch. */
void tasking_switch() {
	if (processes == 0) PANIC("No processes are running !");
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
		if (current_thread->state == TS_RUNNING) sched_enqueue(current_thread);
	}

	current_thread = sched_dequeue();
	ASSERT(current_thread != 0);

	pagedir_switch(current_thread->process->pagedir);

	gdt_setKernelStack(((uint32_t)current_thread->kernelStack_addr) + current_thread->kernelStack_size);

	asm volatile("			\
			mov %0, %%ebp;	\
			mov %1, %%esp;	\
			mov %2, %%ecx;	\
			mov $0x12345, %%eax;	\
			jmp *%%ecx;"
		: : "r"(current_thread->ebp), "r"(current_thread->esp), "r"(current_thread->eip));
}

/*	Called when an exception happens. Provides a stack trace if it was in kernel land.
	Ends the thread for most exceptions, ends the whole process for page faults. */
uint32_t tasking_handleException(struct registers *regs) {
	if (current_thread == 0) return 0;	//No tasking yet
	NL; WHERE; monitor_write("Unhandled exception : ");
	char *exception_messages[] = {"Division By Zero","Debug","Non Maskable Interrupt","Breakpoint",
    "Into Detected Overflow","Out of Bounds","Invalid Opcode","No Coprocessor", "Double Fault",
    "Coprocessor Segment Overrun","Bad TSS","Segment Not Present","Stack Fault","General Protection Fault",
    "Page Fault","Unknown Interrupt","Coprocessor Fault","Alignment Check","Machine Check"};
	monitor_write(exception_messages[regs->int_no]);
	monitor_write(" eip:"); monitor_writeHex(regs->eip);
	if (regs->eip >= 0xE0000000) {
		monitor_write("\n  Stack trace :");
		uint32_t *stack = (uint32_t*)regs->ebp, i;
		for (i = 0; i < 5 && stack > 0xE0000000 && stack < (regs->useresp + 0x8000); i++) {
			monitor_write("\nframe@"); monitor_writeHex(stack);
			monitor_write(" next:"); monitor_writeHex(stack[0]); monitor_write(" ret:"); monitor_writeHex(stack[1]);
			stack = (uint32_t*)stack[0];
		}
		PANIC("Kernel error'd.");
	}
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

/*	Puts the current thread in an inactive state. */
void thread_goInactive() {
	current_thread->state = TS_WAKEWAIT;
	tasking_switch();
}

/*	Wakes up the given thread. */
void thread_wakeUp(struct thread* t) {
	if (t->state == TS_WAKEWAIT) {
		t->state = TS_RUNNING;
		sched_enqueue(t);
	}
}

/*	Returns the privilege level of the current process. */
int proc_priv() {
	if (current_thread == 0 || current_thread->process == 0) return PL_UNKNOWN;
	return current_thread->process->privilege;
}

/*	For internal use only. Called by thread_exit_stackJmp on a stack that will not be deleted.
	Exits current thread or process, depending on the reason. */
void thread_exit2(uint32_t reason) {		//See EX_TH_* defines in task.h
	/*
	 * if reason == EX_TH_NORMAL, it is just one thread exiting because it has to
	 * if reason == EX_TH_EXCEPTION, it is just one thread exiting because of an exception
	 * if reason is none of the two cases above, it is the whole process exiting (with error code = reason)
	 */
	struct thread *th = current_thread;
	if (th == 0 || th->process == 0) goto retrn;
	struct process *pr = th->process;
	if ((reason == EX_TH_NORMAL || reason == EX_TH_EXCEPTION) && pr->thread_count > 1) {
		thread_delete(th);
	} else {
		process_delete(pr);
	}
	retrn:
	sti();
	tasking_switch();
}

/*	For internal use only. Called by thread_exit and process_exit.
	Switches to a stack that will not be deleted when current thread is deleted. */
void thread_exit_stackJmp(uint32_t reason) {
	cli();
	uint32_t *stack;
	stack = tasking_tmpStack + (KSTACKSIZE / 4);
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

/*	System call. Exit the current thread. */
void thread_exit() {
	thread_exit_stackJmp(EX_TH_NORMAL);
}

/*	System call. Exit the current process. */
void process_exit(uint32_t retval) {
	if (retval == EX_TH_NORMAL || retval == EX_TH_EXCEPTION) retval = EX_PR_EXCEPTION;
	thread_exit_stackJmp(retval);
}

/*	For internal use only. This is called when a newly created thread first runs
	(its address is the value given for EIP).
	It switches to user mode if necessary and calls the entry point. */
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

/*	Creates a new thread for given process.
	Allocates a kernel stack and a user stack if necessary.
	Sets up the kernel stack for values to be passed to thread_run. */
struct thread *thread_new(struct process *proc, thread_entry entry_point, void *data) {
	struct thread *t = kmalloc(sizeof(struct thread));
	t->process = proc;
	t->next = 0;
	proc->thread_count++;

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
	sched_enqueue(t);

	if (proc->threads == 0) {
		proc->threads = t;
	} else {
		struct thread *i = proc->threads;
		while (i->next != 0) i = i->next;
		i->next = t;
	}
	return t;
}

/*	Creates a new process. Creates a struct process and fills it up. */
struct process *process_new(struct process* parent, uint32_t uid, uint32_t privilege) {
	struct process* p = kmalloc(sizeof(struct process));
	p->pid = (nextpid++);
	p->uid = uid;
	p->thread_count = 0;
	p->privilege = privilege;
	p->parent = parent;
	p->pagedir = pagedir_new();
	p->next = processes;
	p->stacksBottom = 0xDF000000;
	p->heapseg = 0;

	p->next_objdesc = 0;
	p->objects = 0;
	struct object* o = obj_new(p);
	if (manager_object == 0) manager_object = o;
	objdesc_add(p, o);	//create process' root object and add descriptor 0 to it
	objdesc_add(p, manager_object);

	processes = p;
	return p;
}

/*	Deletes given thread, freeing the stack(s). */
static void thread_delete(struct thread *th) {
	if (th->process->threads == th) {
		th->process->threads = th->next;
	} else {
		struct thread *it = th->process->threads;
		while (it) {
			if (it->next == th) {
				it->next = th->next;
				break;
			}
			it = it->next;
		}
	}
	if (current_thread == th) current_thread = 0;
	th->process->thread_count--;
	kfree(th->kernelStack_addr);
	if (th->userStack_seg != 0) seg_unmap(th->userStack_seg);
	kfree(th);
}

/*	Deletes a process. First, deletes all its threads. Also deletes the corresponding page directory. */
static void process_delete(struct process *pr) {
	struct thread *it = pr->threads;
	while (it != 0) {
		thread_delete(it);
		it = it->next;
	}
	obj_closeall(pr);
	if (processes == pr) {
		processes = pr->next;
	} else {
		struct process *it = processes;
		while (it) {
			if (it->next == pr) {
				it->next = pr->next;
				break;
			}
			it = it->next;
		}
	}
	pagedir_delete(pr->pagedir);
	kfree(pr);
}

/*	System call. Called by the app to define the place for the heap. */
int process_setheapseg(size_t start, size_t end) {		//syscall
	struct process *p = current_thread->process;
	if (start >= 0xE0000000 || end >= 0xE0000000) return -1;
	if (p->heapseg == 0) {
		struct segment *s = simpleseg_make(start, end - start, 1);
		if (s == 0) return -5;
		p->heapseg = seg_map(s, p->pagedir, 0);
		if (p->heapseg == 0) return -1;
		return 0;
	} else if (p->heapseg->start != start) {
		seg_unmap(p->heapseg);
		struct segment *s = simpleseg_make(start, end - start, 1);
		if (s == 0) return -5;
		p->heapseg = seg_map(s, p->pagedir, 0);
		if (p->heapseg == 0) return -1;
		return 0;
	} else {
		return simpleseg_resize(p->heapseg, end - start);
	}
}
