void printk(char *s) {
	asm volatile("int $64" : : "a"(4), "b"(s));
}

void start() {
	printk("Hi world !");
	asm volatile("int $64" : : "a"(0));
}
