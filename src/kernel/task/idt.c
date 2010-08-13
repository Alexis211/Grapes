#include "idt.h"
#include <core/monitor.h>
#include <core/sys.h>
#include <mem/paging.h>
#include "task.h"
#include "syscall.h"

#include <stdlib.h>

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

extern void syscall64();

extern void idt_flush(int32_t ptr);

struct idt_entry idt_entries[256];
struct idt_ptr idt_ptr;

static int_callback irq_handlers[16] = {0};
static struct irq_waiter {
	struct thread *thread;
	struct irq_waiter *next;
} *irq_wakeup[16] = {0};

/*	Called in idt_.asm when an exception fires (interrupt 0 to 31).
	Tries to handle the exception, panics if fails. */
void idt_isrHandler(struct registers regs) {
	if ((regs.int_no == 14 && paging_fault(&regs) != 0) || regs.int_no != 14) {
		if (tasking_handleException(&regs) == 0) {
			monitor_write("\n  >>  >> SOMETHING BAD HAPPENNED << <<\n");
			monitor_write("Unhandled exception ");
			monitor_writeHex(regs.int_no);
			monitor_write(" @"); monitor_writeHex(regs.eip);
			monitor_put('\n');
			PANIC("Unhandled Exception");
		}
	}
} 

/*	Called in idt_.asm when an IRQ fires (interrupt 32 to 47)
	Possibly wakes up a thread that was waiting, possibly calls a handler. */
void idt_irqHandler(struct registers regs) {
	uint32_t doSwitch = (regs.err_code == 0);	//IRQ0 = timer
	if (regs.err_code > 7) {
		outb(0xA0, 0x20);
	}
	outb(0x20, 0x20);
	while (irq_wakeup[regs.err_code] != 0) {
		struct irq_waiter *tmp = irq_wakeup[regs.err_code];
		thread_wakeUp(tmp->thread);
		irq_wakeup[regs.err_code] = tmp->next;
		kfree(tmp);
		doSwitch = 1;
	}
	if (irq_handlers[regs.err_code] != 0) {
		irq_handlers[regs.err_code](&regs);
	}
	if (doSwitch) tasking_switch();
}

/*	Called in idt_.asm on a system call (interrupt 64).
	Calls the correct syscall handler (if any). */
void idt_syscallHandler(struct registers regs) {
	if (regs.eax < NUMBER_OF_SYSCALLS && syscalls[regs.eax] != 0) {
		syscalls[regs.eax](&regs);
	}
}

/*	For internal use only. Sets up an entry of the IDT with given parameters. */
static void idt_setGate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
	idt_entries[num].base_lo = base & 0xFFFF;
	idt_entries[num].base_hi = (base >> 16) & 0xFFFF;

	idt_entries[num].sel = sel;
	idt_entries[num].always0 = 0;
	idt_entries[num].flags = flags | 0x60;
}

/*	Remaps the IRQs. Sets up the IDT. */
void idt_init() {
	idt_ptr.limit = (sizeof(struct idt_entry) * 256) - 1;
	idt_ptr.base = (uint32_t)&idt_entries;

	memset((uint8_t*)&idt_entries, 0, sizeof(struct idt_entry) * 256);

	//Remap the IRQ table
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x0);
	outb(0xA1, 0x0);

	idt_setGate(0, (int32_t)isr0, 0x08, 0x8E);
	idt_setGate(1, (int32_t)isr1, 0x08, 0x8E);
	idt_setGate(2, (int32_t)isr2, 0x08, 0x8E);
	idt_setGate(3, (int32_t)isr3, 0x08, 0x8E);
	idt_setGate(4, (int32_t)isr4, 0x08, 0x8E);
	idt_setGate(5, (int32_t)isr5, 0x08, 0x8E);
	idt_setGate(6, (int32_t)isr6, 0x08, 0x8E);
	idt_setGate(7, (int32_t)isr7, 0x08, 0x8E);
	idt_setGate(8, (int32_t)isr8, 0x08, 0x8E);
	idt_setGate(9, (int32_t)isr9, 0x08, 0x8E);
	idt_setGate(10, (int32_t)isr10, 0x08, 0x8E);
	idt_setGate(11, (int32_t)isr11, 0x08, 0x8E);
	idt_setGate(12, (int32_t)isr12, 0x08, 0x8E);
	idt_setGate(13, (int32_t)isr13, 0x08, 0x8E);
	idt_setGate(14, (int32_t)isr14, 0x08, 0x8E);
	idt_setGate(15, (int32_t)isr15, 0x08, 0x8E);
	idt_setGate(16, (int32_t)isr16, 0x08, 0x8E);
	idt_setGate(17, (int32_t)isr17, 0x08, 0x8E);
	idt_setGate(18, (int32_t)isr18, 0x08, 0x8E);
	idt_setGate(19, (int32_t)isr19, 0x08, 0x8E);
	idt_setGate(20, (int32_t)isr20, 0x08, 0x8E);
	idt_setGate(21, (int32_t)isr21, 0x08, 0x8E);
	idt_setGate(22, (int32_t)isr22, 0x08, 0x8E);
	idt_setGate(23, (int32_t)isr23, 0x08, 0x8E);
	idt_setGate(24, (int32_t)isr24, 0x08, 0x8E);
	idt_setGate(25, (int32_t)isr25, 0x08, 0x8E);
	idt_setGate(26, (int32_t)isr26, 0x08, 0x8E);
	idt_setGate(27, (int32_t)isr27, 0x08, 0x8E);
	idt_setGate(28, (int32_t)isr28, 0x08, 0x8E);
	idt_setGate(29, (int32_t)isr29, 0x08, 0x8E);
	idt_setGate(30, (int32_t)isr30, 0x08, 0x8E);
	idt_setGate(31, (int32_t)isr31, 0x08, 0x8E);

	idt_setGate(32, (int32_t)irq0, 0x08, 0x8E);
	idt_setGate(33, (int32_t)irq1, 0x08, 0x8E);
	idt_setGate(34, (int32_t)irq2, 0x08, 0x8E);
	idt_setGate(35, (int32_t)irq3, 0x08, 0x8E);
	idt_setGate(36, (int32_t)irq4, 0x08, 0x8E);
	idt_setGate(37, (int32_t)irq5, 0x08, 0x8E);
	idt_setGate(38, (int32_t)irq6, 0x08, 0x8E);
	idt_setGate(39, (int32_t)irq7, 0x08, 0x8E);
	idt_setGate(40, (int32_t)irq8, 0x08, 0x8E);
	idt_setGate(41, (int32_t)irq9, 0x08, 0x8E);
	idt_setGate(42, (int32_t)irq10, 0x08, 0x8E);
	idt_setGate(43, (int32_t)irq11, 0x08, 0x8E);
	idt_setGate(44, (int32_t)irq12, 0x08, 0x8E);
	idt_setGate(45, (int32_t)irq13, 0x08, 0x8E);
	idt_setGate(46, (int32_t)irq14, 0x08, 0x8E);
	idt_setGate(47, (int32_t)irq15, 0x08, 0x8E);

	idt_setGate(64, (int32_t)syscall64, 0x08, 0x8E);

	idt_flush((int32_t)&idt_ptr);

	monitor_write("[IDT] ");
}

/*	Sets up an IRQ handler for given IRQ. */
void idt_handleIrq(int number, int_callback func) {
	if (number < 16 && number >= 0) {
		irq_handlers[number] = func;
	}
}

/*	Tells the IRQ handler to wake up the current thread when specified IRQ fires. */
void idt_waitIrq(int number) {
	if (number < 16 && number >= 0 && proc_priv() <= PL_DRIVER) {
		struct irq_waiter *tmp = kmalloc(sizeof(struct irq_waiter));
		tmp->thread = current_thread;
		tmp->next = irq_wakeup[number];
		irq_wakeup[number] = tmp;

		thread_goInactive();
	}
}
