#include <types.h>
#include "multiboot.h"
#include "monitor.h"
#include "sys.h"
#include <task/idt.h>
#include <task/timer.h>
#include <task/task.h>
#include <mem/gdt.h>
#include <mem/paging.h>
#include <mem/mem.h>
#include <linker/elf.h>

void kmain(struct multiboot_info_t* mbd, int32_t magic) {
	size_t totalRam = 0;
	uint32_t i;

	mem_placementAddr = (size_t)&end;
	mbd->cmdline += 0xE0000000; mbd->mods_addr += 0xE0000000;
	struct module_t *mods = (struct module_t*)mbd->mods_addr;
	for (i = 0; i < mbd->mods_count; i++) {
		mods[i].mod_start += 0xE0000000;
		mods[i].mod_end += 0xE0000000;
		mods[i].string += 0xE0000000;
		if (mods[i].mod_end > mem_placementAddr)
			mem_placementAddr = (mods[i].mod_end & 0xFFFFF000) + 0x1000;
	}

	monitor_clear();

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		PANIC("wrong multiboot magic number.");
	}

	monitor_write("Grapes kernel booting ...\n");

	idt_init();

	totalRam = ((mbd->mem_upper + mbd->mem_lower) * 1024);
	paging_init(totalRam);
	gdt_init();
	paging_cleanup();
	kheap_init();
	timer_init(20);
	tasking_init();
	
	monitor_write("Loading modules...\n");
	for (i = 0; i < mbd->mods_count; i++) {
		monitor_write((char*)mods[i].string);
		if (elf_check((uint8_t*)mods[i].mod_start)) {
			monitor_write(" : Invalid ELF file\n");
		} else {
			if (elf_exec((uint8_t*)mods[i].mod_start) == 0) {
				monitor_write(" : Error loading\n");
			} else {
				monitor_write(" : OK\n");
			}
		}
	}

	monitor_write("Passing control to loaded modules...\n");
	sti();
	tasking_switch();
}
