
CC = gcc
CFLAGS = -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -fno-stack-protector -Wall -Wextra

LD = ld
LDFLAGS = -T link.ld

ASM = nasm
AFLAGS = -f elf

OBJECTS = loader_.o kmain.o sys.o \
		  monitor.o timer.o \
		  idt.o idt_.o task.o task_.o\
		  lib/stdlib.o lib/bitset.o \
		  mem.o paging.o gdt.o heap.o
OUT = stem.elf

all: $(OBJECTS)
	echo "* Linking $(OUT)..."
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(OUT) -Map stem.map

clean:
	rm *.o || exit 0
	rm $(OBJECTS) || exit 0

mrproper: clean
	rm $(OUT) || exit 0

%.o: %.asm
	echo "* Compiling $<..."
	$(ASM) $(AFLAGS) -o $@ $<
	
%.o: %.c
	echo "* Compiling $<..."
	$(CC) -c $< -o $@ $(CFLAGS)
