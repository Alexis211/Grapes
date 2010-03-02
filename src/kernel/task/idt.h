#ifndef DEF_IDT_H
#define DEF_IDT_H

#include <types.h>

struct idt_entry {
	uint16_t base_lo;		//Low part of address to jump to
	uint16_t sel;			//Kernel segment selector
	uint8_t always0;
	uint8_t flags;			//Flags
	uint16_t base_hi;		//High part of address to jump to
} __attribute__((packed));

struct idt_ptr {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

struct registers {
	uint32_t cr3;		//page directory physical address
	uint32_t ds;                  // Data segment selector
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
	uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
	uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
};

typedef void (*int_callback)(struct registers*);

void idt_init();
void idt_handleIrq(int number, int_callback func);	//Set IRQ handler
void idt_waitIrq(int number);	//ask current thread to wait for IRQ

#endif

