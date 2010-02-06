#include "gdt.h"
#include <stdlib.h>
#include <core/monitor.h>

extern void gdt_flush(uint32_t);	//ASM (idt_.asm)

#define GDT_ENTRIES 5

static struct gdt_entry gdt_entries[GDT_ENTRIES];
static struct gdt_ptr gdt_ptr;

static void gdt_setGate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
	gdt_entries[num].base_low = (base & 0xFFFF);
	gdt_entries[num].base_middle = (base >> 16) & 0xFF;
	gdt_entries[num].base_high = (base >> 24) & 0xFF;

	gdt_entries[num].limit_low = (limit & 0xFFFF);
	gdt_entries[num].granularity = (limit >> 16) & 0x0F;
	gdt_entries[num].granularity |= gran & 0xF0;
	gdt_entries[num].access = access;
}

void gdt_init() {
	gdt_ptr.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
	gdt_ptr.base = (uint32_t)&gdt_entries;

	gdt_setGate(0, 0, 0, 0, 0);					//Null segment
	gdt_setGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);	//Kernel code segment
	gdt_setGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);	//Kernel data segment
	gdt_setGate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);	//User code segment
	gdt_setGate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);	//User data segment

	gdt_flush((uint32_t)&gdt_ptr);

	monitor_write("GDT ok\n");
}
