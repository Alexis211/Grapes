#include "syscall.h"
#include "task.h"

#define CALL0(name, scname) static void scname(struct registers* r) { r->eax = name(); }
#define CALL1(name, scname) static void scname(struct registers* r) { \
	r->eax = name(r->ebx); }
#define CALL2(name, scname) static void scname(struct registers* r) { \
	r->eax = name(r->ebx, r->ecx); }
#define CALL3(name, scname) static void scname(struct registers* r) { \
	r->eax = name(r->ebx, r->ecx, r->edx); }
#define CALL0V(name, scname) static void scname(struct registers* r) { name(); }
#define CALL1V(name, scname) static void scname(struct registers* r) { name(r->ebx); }
#define CALL2V(name, scname) static void scname(struct registers* r) { name(r->ebx, r->ecx); }
#define CALL3V(name, scname) static void scname(struct registers* r) { name(r->ebx, r->ecx, r->edx); }

CALL0V(thread_exit, thread_exit_sc);
CALL0V(tasking_switch, schedule_sc);
CALL1V(thread_sleep, thread_sleep_sc);
CALL1V(process_exit, process_exit_sc);
CALL1(monitor_write, printk_sc);
CALL1V(idt_waitIrq, irq_wait_sc);
CALL0(proc_priv, proc_priv_sc);
CALL2(shm_create, shm_create_sc);
CALL1(shm_delete, shm_delete_sc);
CALL0(object_create, object_create_sc);
CALL1(object_owned, object_owned_sc);
CALL1V(object_close, object_close_sc);
CALL3(request_get, request_get_sc);
CALL1(request_has, request_has_sc);
CALL3V(request_answer, request_answer_sc);
CALL3(request_mapShm, request_mapShm_sc);
CALL2(request, request_sc);
CALL2(send_msg, send_msg_sc);

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
	object_create_sc,		//10
	object_owned_sc,
	object_close_sc,
	request_get_sc,
	request_has_sc,
	request_answer_sc,		//15
	request_mapShm_sc,
	request_sc,
	send_msg_sc,
	0 };
