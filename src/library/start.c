extern int main();

void start() {
	int ret = main();
	asm volatile("int $64" : : "a"(3), "b"(ret));
}
