#include "sys.h"
#include "monitor.h"

void outb(uint16_t port, uint8_t value) {
	asm volatile("outb %1, %0" : : "dN"(port), "a"(value));
}

void outw(uint16_t port, uint16_t value) {
	asm volatile("outw %1, %0" : : "dN"(port), "a"(value));
}

uint8_t inb(uint16_t port) {
	uint8_t ret;
	asm volatile("inb %1, %0" : "=a"(ret) : "dN"(port));
	return ret;
}

uint16_t inw(uint16_t port) {
	uint16_t ret;
	asm volatile("inw %1, %0" : "=a"(ret) : "dN"(port));
	return ret;
}

void panic(char* message, char* file, int line) {
	monitor_write(">> PANIC: >>");
	monitor_write(message); monitor_write("<< in file "); monitor_write(file);
	monitor_write("\nSystem halted T_T");
	asm volatile("cli; hlt");
}

static uint32_t if_locks = 1;

void cli() {
	asm volatile("cli");
	if_locks++;
}

void sti() {
	if (if_locks > 0) if_locks--;
	if (if_locks == 0) asm volatile("sti");
}
