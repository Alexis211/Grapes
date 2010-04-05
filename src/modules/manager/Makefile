.PHONY: clean, mrproper

CC = gcc
CFLAGS = -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -fno-stack-protector -Wall -Wextra -I ../../include

LD = ld
LDFLAGS = -T ../../library/link.ld -L ../../library -Map manager.map

Objects = main.o
Outfile = manager.elf

all: $(Outfile)
	echo "* Done with $(Outfile)"

rebuild: mrproper all

$(Outfile): $(Objects)
	echo "* Linking $@..."
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	echo "* Removing objects..."
	rm *.o || exit 0
	rm *.map || exit 0
	rm $(Objects) || exit 0

mrproper: clean
	rm *.elf || exit 0
	rm $(Outfile) || exit 0

