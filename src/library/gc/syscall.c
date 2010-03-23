#include <gc/syscall.h>

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

void thread_new(void (*entry)(void*), void *data) {
	call(5, (unsigned)entry, (unsigned)data, 0, 0, 0);
}

void irq_wait(int number) {
	call(6, number, 0, 0, 0, 0);
}

int proc_priv() {
	return call(7, 0, 0, 0, 0, 0);
}

int shm_create(size_t offset, size_t length) {
	return call(8, offset, length, 0, 0, 0);
}

int shm_delete(size_t offset) {
	return call(9, offset, 0, 0, 0, 0);
}
