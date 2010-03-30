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

int object_create() {
	return call(10, 0, 0, 0, 0, 0);
}

int object_owned(int descriptor) {
	return call(11, descriptor, 0, 0, 0, 0);
}

void object_close(int descriptor) {
	call(12, descriptor, 0, 0, 0, 0);
}

int request_get(int descriptor, struct user_request *rq, int wait) {
	return call(13, descriptor, (size_t)rq, wait, 0, 0);
}

int request_has(int descriptor) {
	return call(14, descriptor, 0, 0, 0, 0);
}

void request_answer(int descriptor, uint32_t answer1, uint32_t answer2, int errcode) {
	call(15, descriptor, answer1, answer2, errcode, 0);
}

int request_mapShm(int descriptor, size_t offset, int number) {
	return call(16, descriptor, offset, number, 0, 0);
}

int request(int descriptor, struct user_sendrequest *rq) {
	return call(17, descriptor, (size_t)rq, 0, 0, 0);
}

int send_msg(int descriptor, struct user_sendrequest *rq) {
	return call(18, descriptor, (size_t)rq, 0, 0, 0);
}

int proc_setheap(size_t start, size_t end) {
	return call(19, start, end, 0, 0, 0);
}
