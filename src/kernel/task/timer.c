#include "timer.h"
#include "task.h"
#include "idt.h"
#include <core/sys.h>
#include <core/monitor.h>

static uint32_t tick = 0, frequency = 0, uptime = 0;

static void timer_callback(struct registers *regs);
static void timer_wakeUpSleepingThreads();

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

/*	Accessor function to get machine uptime. */
uint32_t timer_uptime() { return uptime; }

/*	Accessor function, gets uptime in miliseconds. */
uint32_t timer_time() {
	return (uptime * 1000) + (tick * 1000 / frequency);
}

/*	Called when IRQ0 fires. Updates the uptime variable.
	DOES NOT provoke a task switch. The task switch is called in idt.c (IRQ handler). */
void timer_callback(struct registers *regs) {
	tick++;
	if (tick == frequency) {
		uptime++;
		tick = 0;
	}
	timer_wakeUpSleepingThreads();
}

//************************************************	SLEEP FUNCTIONS *****************

static struct sleeping_thread {
	uint32_t wakeup_time;
	struct thread *thread;
	struct sleeping_thread *next;
} *sleeping_threads = 0;

/*	Makes the current thread sleep. */
void thread_sleep(uint32_t msecs) {
	if (current_thread == 0) return;
	// Create the sleeping_thread structure
	struct sleeping_thread *sf = kmalloc(sizeof(struct sleeping_thread)), *tmp;
	sf->wakeup_time = timer_time() + msecs;
	sf->thread = current_thread;
	//Insert it at the right place
	if (sleeping_threads == 0 || sleeping_threads->wakeup_time >= sf->wakeup_time) {
		sf->next = sleeping_threads;
		sleeping_threads = sf;
	} else {
		tmp = sleeping_threads;
		while (1) {
			if (tmp->next == 0 || tmp->next->wakeup_time >= sf->wakeup_time) {
				sf->next = tmp->next;
				tmp->next = sf;
				break;
			}
			tmp = tmp->next;
		}
	}

	thread_goInactive();
}

void timer_wakeUpSleepingThreads() {
	uint32_t time = timer_time();
	while (sleeping_threads != 0 && sleeping_threads->wakeup_time <= time) {
		struct sleeping_thread *tmp = sleeping_threads;
		thread_wakeUp(tmp->thread);
		sleeping_threads = tmp->next;
		kfree(tmp);
	}
}
