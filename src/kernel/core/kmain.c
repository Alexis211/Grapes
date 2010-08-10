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

/*	The kernel's main procedure. This function is called in loader_.asm.
	This function calls the initializer functions for all system parts.
	It then loads the modules the kernel was given by the bootloader. 
	This function never returns : once multitasking is started for good,
	the execution flow of this function is never returned to. */
void kmain(struct multiboot_info_t* mbd, int32_t magic) {
	monitor_clear();

	ASSERT(magic == MULTIBOOT_BOOTLOADER_MAGIC);

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

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		PANIC("wrong multiboot magic number.");
	}

	monitor_write("Grapes 0.0.4 'Cat in my heart' starting up :\n");

	idt_init();

	totalRam = ((mbd->mem_upper + mbd->mem_lower) * 1024);
	paging_init(totalRam);
	gdt_init();
	paging_cleanup();
	kheap_init();
	timer_init(15);
	tasking_init();
	
	monitor_write("\nLoading modules :\n");
	for (i = 0; i < mbd->mods_count; i++) {
		monitor_write(" * ");
		monitor_write((char*)mods[i].string);
		if (elf_check((uint8_t*)mods[i].mod_start)) {
			monitor_write(" : Invalid ELF file\n");
		} else {
			struct process *pr = elf_exec((uint8_t*)mods[i].mod_start, PL_DRIVER);
			if (pr == 0) {
				monitor_write(" : Error loading\n");
			} else {
				monitor_write(" : OK pid:"); monitor_writeDec(pr->pid); monitor_write("\n");
			}
		}
	}

	monitor_write("Modules now RULE THE WORLD !\n");
	sti();
	tasking_switch();
	PANIC("Should never happen. Something probably went wrong with multitasking.");
}
