#ifndef DEF_ELF_H
#define DEF_ELF_H

#include <types.h>
#include <task/task.h>

/* elf_phdr_t :: p_type : program header entries types */
#define	PT_NULL             0
#define	PT_LOAD             1
#define	PT_DYNAMIC          2
#define	PT_INTERP           3
#define	PT_NOTE             4
#define	PT_SHLIB            5
#define	PT_PHDR             6
#define	PT_LOPROC  0x70000000
#define	PT_HIPROC  0x7fffffff
 
/* elf_phdr_t :: p_flags : program header entries flags */
#define PF_X	(1 << 0)
#define PF_W	(1 << 1)
#define PF_R	(1 << 2)

struct elf_ehdr {
	uint8_t e_ident[16];      /* ELF identification */
	uint16_t e_type;             /* 2 (exec file) */
	uint16_t e_machine;          /* 3 (intel architecture) */
	uint32_t e_version;          /* 1 */
	uint32_t e_entry;            /* starting point */
	uint32_t e_phoff;            /* program header table offset */
	uint32_t e_shoff;            /* section header table offset */
	uint32_t e_flags;            /* various flags */
	uint16_t e_ehsize;           /* ELF header (this) size */

	uint16_t e_phentsize;        /* program header table entry size */
	uint16_t e_phnum;            /* number of entries */

	uint16_t e_shentsize;        /* section header table entry size */
	uint16_t e_shnum;            /* number of entries */

	uint16_t e_shstrndx;         /* index of the section name string table */
};
 
struct elf_phdr {
	uint32_t p_type;             /* type of segment */
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
};
 
struct phdr {
	struct elf_phdr h;
	uint8_t* data;
};

int elf_check(uint8_t *data);	//0 if ok, 1 if not a valid ELF
thread_entry elf_load(uint8_t *data, struct process* process);	//Load an ELF to a process, return entry point
struct process* elf_exec(uint8_t *data, int privilege);	//Creates a new process and a thread for running ELF file

#endif
