#include "timer.h"
#include "idt.h"
#include <core/sys.h>
#include <core/monitor.h>

static uint32_t tick = 0, frequency = 0, uptime = 0;

/*	Called when IRQ0 fires. Updates the uptime variable.
	DOES NOT provoke a task switch. The task switch is called in idt.c (IRQ handler). */
void timer_callback(struct registers *regs) {
	tick++;
	if (tick == frequency) {
		uptime++;
		tick = 0;
	}
}

/*	Accessor function to get machine uptime. */
uint32_t timer_uptime() { return uptime; }

/*	Accessor function, gets uptime in miliseconds. */
uint32_t timer_time() {
	return (uptime * 1000) + (tick * 1000 / frequency);
}

/*	Called by kmain. Sets up the PIT and the IRQ0 handler. */
void timer_init(uint32_t freq) {
	frequency = freq;

	idt_handleIrq(0, timer_callback);

	uint32_t divisor = 1193180 / freq;

	outb(0x43, 0x36);	//Command byte

	uint8_t l = (divisor & 0xFF), h = (divisor >> 8);
	outb(0x40, l);
	outb(0x40, h);

	monitor_write("[PIT] ");
}
