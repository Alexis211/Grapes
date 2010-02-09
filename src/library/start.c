#include "grapes/syscall.h"

extern int main();

void start() {
	int ret = main();
	process_exit(ret);
}
