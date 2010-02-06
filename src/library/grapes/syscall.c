#include "syscall.h"

static int call(unsigned a, unsigned b, unsigned c, unsigned d, unsigned e, unsigned f) {
	unsigned ret;
	asm volatile("int $64" : "=a"(ret) : "a"(a), "b"(b), "c"(c), "d"(d), "S"(e), "D"(f));
	return ret;
}

void thread_exit() {
	call(0, 0, 0, 0, 0, 0);
}

void schedule() {
	call(1, 0, 0,0, 0, 0);
}

void thread_sleep(int time) {
	call(2, time, 0, 0, 0, 0);
}

void process_exit(int retval) {
	call(3, retval, 0, 0, 0, 0);
}

void printk(char* str) {
	call(4, (unsigned)str, 0, 0, 0, 0);
}
