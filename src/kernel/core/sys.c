#include "sys.h"
#include "monitor.h"

/*	These four functions are wrappers around ASM operations.
	These functions are used when comunicating with the system hardware. */

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

/*	These two functions stop the system, reporting an error message, because something bad happenned. */

void panic(char* message, char* file, int line) {
	monitor_write("\n\nPANIC:\t\t"); monitor_write(message);
	monitor_write("\n File:\t\t"); monitor_write(file); monitor_put(':'); monitor_writeDec(line);
	monitor_write("\n\t\tSystem halted -_-'\n");
	asm volatile("cli; hlt");
}

void panic_assert(char* assertion, char* file, int line) {
	monitor_write("\n\nASSERT FAILED:\t"); monitor_write(assertion);
	monitor_write("\n File:\t\t"); monitor_write(file); monitor_put(':'); monitor_writeDec(line);
	monitor_write("\n\t\tSystem halted -_-'\n");
	asm volatile("cli; hlt");
}

/* Global system mutex. See comments in sys.h. */

static uint32_t if_locks = 1;

void cli() {
	asm volatile("cli");
	if_locks++;
}

void sti() {
	if (if_locks > 0) if_locks--;
	if (if_locks == 0) asm volatile("sti");
}
