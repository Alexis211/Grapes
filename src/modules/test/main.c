#include <grapes/syscall.h>

int main() {
	printk("[module:test] Hi world !\n");
	thread_sleep(5000);
	printk("[module:test] 5sec later...\n");
	return 0;
}
