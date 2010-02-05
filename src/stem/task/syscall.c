#include "syscall.h"

#define CALL0(name, scname) static void scname(struct registers* r) { r->eax = name(); }
#define CALL1(name, scname) static void scname(struct registers* r) { \
	r->eax = name(r->ebx); }
#define CALL2(name, scname) static void scname(struct registers* r) { \
	r->eax = name(r->ebx, r->ecx); }

CALL0(tasking_switch, schedule_sc);
CALL1(monitor_write, printk_sc);

int_callback syscalls[] = {
	0,			//Syscall 0 will be thread_exit
	schedule_sc,
	0,			//Syscall 2 will be thread_sleep
	0,			//Syscall 3 will be process_exit
	printk_sc,
	0 };
