#include <gc/syscall.h>

#define FACTOR 4

void thread2(void* d) {
	while (1) {
		printk("$");
		thread_sleep(35*FACTOR);
	}
}

int main() {
	printk("[module:test] Hi world !\n");
	printk("[module:test] Creating new thread...\n");
	thread_new(thread2, 0);
	while (1) {
		printk(".");
		thread_sleep(50*FACTOR);
	}
	return 0;
}
