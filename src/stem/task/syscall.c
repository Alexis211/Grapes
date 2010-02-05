#include "syscall.h"

#define CALL0(name, scname) static void scname(struct registers* r) { r->eax = name(); }
#define CALL1(name, scname) static void scname(struct registers* r) { \
	r->eax = name(r->ebx); }
#define CALL2(name, scname) static void scname(struct registers* r) { \
	r->eax = name(r->ebx, r->ecx); }

CALL0(thread_exit, thread_exit_sc);
CALL0(tasking_switch, schedule_sc);
CALL1(thread_sleep, thread_sleep_sc);
CALL1(process_exit, process_exit_sc);
CALL1(monitor_write, printk_sc);

int_callback syscalls[] = {
	thread_exit_sc,
	schedule_sc,
	thread_sleep_sc,
	process_exit_sc,
	printk_sc,
	0 };
