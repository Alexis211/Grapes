#include <gc/syscall.h>

extern int main();

void start() {
	objsrv_init();
	int ret = main();
	process_exit(ret);
}
