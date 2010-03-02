.PHONY: clean, mrproper

CC = gcc
CFLAGS = -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -fno-stack-protector -Wall -Wextra -I . -I ./lib

LD = ld
LDFLAGS = -T link.ld

ASM = nasm
AFLAGS = -f elf

OBJECTS = core/loader_.o core/kmain.o core/sys.o \
		  core/monitor.o task/timer.o \
		  task/idt.o task/idt_.o task/task.o task/task_.o task/syscall.o \
		  lib/stdlib.o lib/bitset.o lib/mutex.o \
		  mem/mem.o mem/paging.o mem/gdt.o mem/heap.o mem/seg.o \
		  ipc/shm.o \
		  linker/elf.o
OUT = kernel.elf

all: $(OBJECTS)
	echo "* Linking $(OUT)..."
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(OUT) -Map kernel.map

clean:
	rm $(OBJECTS) || exit 0
	rm *.o */*.o || exit 0
	rm *.map || exit 0

mrproper: clean
	rm $(OUT) || exit 0

%.o: %.asm
	echo "* Compiling $<..."
	$(ASM) $(AFLAGS) -o $@ $<
	
%.o: %.c
	echo "* Compiling $<..."
	$(CC) -c $< -o $@ $(CFLAGS)
