#include "syscall.h"
#include "task.h"

#define CALL0(name, scname) static void scname(struct registers* r) { r->eax = name(); }
#define CALL1(name, scname) static void scname(struct registers* r) { \
	r->eax = name(r->ebx); }
#define CALL2(name, scname) static void scname(struct registers* r) { \
	r->eax = name(r->ebx, r->ecx); }
#define CALL0V(name, scname) static void scname(struct registers* r) { name(); }
#define CALL1V(name, scname) static void scname(struct registers* r) { name(r->ebx); }
#define CALL2V(name, scname) static void scname(struct registers* r) { name(r->ebx, r->ecx); }

CALL0V(thread_exit, thread_exit_sc);
CALL0V(tasking_switch, schedule_sc);
CALL1V(thread_sleep, thread_sleep_sc);
CALL1V(process_exit, process_exit_sc);
CALL1(monitor_write, printk_sc);
CALL1V(idt_waitIrq, irq_wait_sc);
CALL0(proc_priv, proc_priv_sc);
CALL2(shm_create, shm_create_sc);
CALL1(shm_delete, shm_delete_sc);

static void thread_new_sc(struct registers* r) {
	thread_new(current_thread->process, (thread_entry)r->ebx, (void*)r->ecx);
}

int_callback syscalls[] = {
	thread_exit_sc,			//0
	schedule_sc,
	thread_sleep_sc,
	process_exit_sc,
	printk_sc,
	thread_new_sc,			//5
	irq_wait_sc,
	proc_priv_sc,
	shm_create_sc,
	shm_delete_sc,
	0 };
