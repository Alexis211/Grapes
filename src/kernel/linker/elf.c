#include "elf.h"
#include <mem/paging.h>
#include <mem/seg.h>
#include <stdlib.h>
#include <core/sys.h>

int elf_check(uint8_t *data) {
	struct elf_ehdr *h = (struct elf_ehdr*)data;
	if (h->e_ident[0] == 0x7F && h->e_ident[1] == 'E' && h->e_ident[2] == 'L' && h->e_ident[3] == 'F') return 0;
	return 1;
}

thread_entry elf_load(uint8_t *data, struct process* process) {
	struct elf_ehdr *ehdr = (struct elf_ehdr*)data;
	struct elf_phdr *phdr;
	int i;
	if (elf_check(data)) return 0;

	struct page_directory *r = current_pagedir;
	cli();
	pagedir_switch(process->pagedir);

	phdr = (struct elf_phdr*)((uint8_t*)(data + ehdr->e_phoff));
	for (i = 0; i < ehdr->e_phnum; i++) {
		if (phdr[i].p_type == PT_LOAD) {
			seg_map(simpleseg_make(phdr[i].p_vaddr, phdr[i].p_memsz, (phdr[i].p_flags & PF_W) != 0), process->pagedir, 0);
			memcpy((uint8_t*)phdr[i].p_vaddr, data + phdr[i].p_offset, phdr[i].p_filesz);
			if (phdr[i].p_memsz > phdr[i].p_filesz) {
				memset((uint8_t*)phdr[i].p_vaddr + phdr[i].p_memsz, 0, phdr[i].p_memsz - phdr[i].p_filesz);
			}
		}
	}

	pagedir_switch(r);
	sti();

	return (thread_entry)ehdr->e_entry;
}

struct process* elf_exec(uint8_t *data, int privilege) {
	if (elf_check(data)) return 0;
	
	struct process* p = process_new(0, 0, privilege);

	thread_entry e = elf_load(data, p);

	thread_new(p, e, 0);

	return p;
}
