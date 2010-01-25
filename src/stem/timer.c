#include "timer.h"
#include "idt.h"
#include "sys.h"
#include "monitor.h"

static uint32_t tick = 0, frequency = 0, uptime = 0;

void timer_callback(struct registers *regs) {
	tick++;
	if (tick == frequency) {
		uptime++;
		tick = 0;
	}
}

uint32_t timer_uptime() { return uptime; }

uint32_t timer_time() {
	return (uptime * 1000) + (tick * 1000 / frequency);
}

void timer_init(uint32_t freq) {
	frequency = freq;

	idt_handleIrq(0, timer_callback);

	uint32_t divisor = 1193180 / freq;

	outb(0x43, 0x36);	//Command byte

	uint8_t l = (divisor & 0xFF), h = (divisor >> 8);
	outb(0x40, l);
	outb(0x40, h);

	monitor_write("Timer started\n");
}
