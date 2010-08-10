Out = kernel.elf
Obj = core/loader_.o core/kmain.o core/sys.o \
	  core/monitor.o task/timer.o \
	  task/idt.o task/idt_.o task/task.o task/task_.o task/syscall.o task/sched.o \
	  lib/stdlib.o lib/bitset.o lib/mutex.o \
	  mem/mem.o mem/paging.o mem/gdt.o mem/heap.o mem/seg.o \
	  ipc/shm.o ipc/object.o ipc/request.o \
	  linker/elf.o

include ../common.make

CFLAGS += -I . -I ./lib -g

LDFLAGS += -T link.ld -Map kernel.map

