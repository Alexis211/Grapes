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

//////
void stack_trace(size_t bp) {
	uint32_t *stack = (uint32_t*)bp, i;
	for (i = 0; i < 5 && stack > 0xE0000000 && stack < (bp + 0x8000); i++) {
		monitor_write(" "); monitor_writeHex(stack);
		monitor_write("\tnext:"); monitor_writeHex(stack[0]); monitor_write("\t\tret:"); monitor_writeHex(stack[1]);
		monitor_write("\n");
		stack = (uint32_t*)stack[0];
	}
}

/*	For internal use only. Used by panic and panic_assert. */
static void panic_do(char* file, int line) {
	monitor_write("\n File:\t\t"); monitor_write(file); monitor_put(':'); monitor_writeDec(line);
	monitor_write("\nTrace:\n");
	size_t bp; asm volatile("mov %%ebp,%0" : "=r"(bp)); stack_trace(bp);
	monitor_write("\n\t\tSystem halted -_-'\n");
	asm volatile("cli; hlt");
}

/*	These functions stop the system, reporting an error message, because something bad happenned. */
void panic(char* message, char* file, int line) {
	monitor_write("\n\nPANIC:\t\t"); monitor_write(message);
	panic_do(file, line);
}

void panic_assert(char* assertion, char* file, int line) {
	monitor_write("\n\nASSERT FAILED:\t"); monitor_write(assertion);
	panic_do(file, line);
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
