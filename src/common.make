# ============== ENVIRONMENT VARIABLES

CC = i586-elf-gcc
CFLAGS = -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -fno-stack-protector -Wall -Wextra

LD = i586-elf-ld
.PHONY: clean, mrproper

LDFLAGS = 

ASM = nasm
AFLAGS = -f elf

# ============== GENERAL BUILD PROCEDURES

all: $(Out)

$(Out): $(Obj)
	echo ""; echo "- Linking $@..."
	$(LD) $(LDFLAGS) $^ -o $@

# ============== GENERAL CLEAINING PROCEDURES

clean:
	rm $(Obj) || exit 0
	rm *.o */*.o || exit 0

mrproper: clean
	rm $(Out) || exit 0

# ============== SOURCE FILE BUILD PROCEDURES

%.o: %.asm
	echo ""; echo "- $<"
	$(ASM) $(AFLAGS) -o $@ $<
	
%.o: %.c
	echo ""; echo "- $<"
	$(CC) -c $< -o $@ $(CFLAGS)
