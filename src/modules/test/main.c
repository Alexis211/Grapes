#include <grapes/syscall.h>

void thread2(void* d) {
	while (1) {
		thread_sleep(1400);
		printk("$");
	}
}

int main() {
	printk("[module:test] Hi world !\n");
	printk("[module:test] Creating new thread...\n");
	thread_new(thread2, 0);
	while (1) {
		thread_sleep(2000);
		printk(".");
	}
	return 0;
}
