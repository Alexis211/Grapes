#include <grapes/syscall.h>

int main() {
	printk("[module:test] Hi world !\n");
	thread_sleep(2000);
	printk("[module:test] 2sec later...\n");
	printk("[module:test] Performing illegal read in kernel space...\n");
	int *a = (int*)0xE0000004;
	if (*a == 0) printk("is null...\n");
	printk("[module:test] HAHA !!!!\n");
	return 0;
}
