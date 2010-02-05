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

void kmain_othertask(void *data) {
	uint32_t i;
	for(i = 0; i < 100; i++) {
		monitor_write("2task ");
		thread_sleep(0);
	}
	process_exit(0);
}

void kmain_stage2(void *data) {
	sti();
	thread_new(current_thread->process, kmain_othertask, 0);
	while (1) {
		monitor_write("TASK1 ");
		thread_sleep(0);
	}
}

void kmain(struct multiboot_info_t* mbd, int32_t magic) {
	size_t totalRam = 0;

	mem_placementAddr = (size_t)&end;

	monitor_clear();

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		PANIC("wrong multiboot magic number.");
	}

	monitor_write("Grapes::Stem is booting\n");

	idt_init();

	totalRam = ((mbd->mem_upper + mbd->mem_lower) * 1024);
	paging_init(totalRam);
	gdt_init();
	paging_cleanup();
	kheap_init();
	timer_init(20);
	tasking_init(kmain_stage2, 0);
}
