#include "gdt.h"
#include <stdlib.h>
#include <core/monitor.h>

extern void gdt_flush(uint32_t);	//ASM (idt_.asm)
extern void tss_flush();

#define GDT_ENTRIES 6

static struct tss_entry tss_entry;
static struct gdt_entry gdt_entries[GDT_ENTRIES];
static struct gdt_ptr gdt_ptr;

void gdt_setKernelStack(uint32_t esp0) {
	tss_entry.esp0 = esp0;
}

static void gdt_setGate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
	gdt_entries[num].base_low = (base & 0xFFFF);
	gdt_entries[num].base_middle = (base >> 16) & 0xFF;
	gdt_entries[num].base_high = (base >> 24) & 0xFF;

	gdt_entries[num].limit_low = (limit & 0xFFFF);
	gdt_entries[num].granularity = (limit >> 16) & 0x0F;
	gdt_entries[num].granularity |= gran & 0xF0;
	gdt_entries[num].access = access;
}

static void gdt_writeTss(int num, uint32_t ss0, uint32_t esp0) {
	uint32_t base = (uint32_t)&tss_entry;
	uint32_t limit = base + sizeof(struct tss_entry);

	gdt_setGate(num, base, limit, 0xE9, 0);

	memset(&tss_entry, 0, sizeof(struct tss_entry));

	tss_entry.ss0 = ss0; tss_entry.esp0 = esp0;

	tss_entry.cs = 0x0b;
	tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13;
}

void gdt_init() {
	gdt_ptr.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
	gdt_ptr.base = (uint32_t)&gdt_entries;

	gdt_setGate(0, 0, 0, 0, 0);					//Null segment
	gdt_setGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);	//Kernel code segment
	gdt_setGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);	//Kernel data segment
	gdt_setGate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);	//User code segment
	gdt_setGate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);	//User data segment
	gdt_writeTss(5, 0x10, 0);

	gdt_flush((uint32_t)&gdt_ptr);
	tss_flush();

	monitor_write("[GDT] ");
}
