.PHONY: clean, mrproper

CC = gcc
CFLAGS = -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -fno-stack-protector -Wall -Wextra -I../include

LD = ld
LDFLAGS = -r

Library = grapes.o
Objects = gc/syscall.o \
		  start.o

all: $(Library)
	echo "* Done with library $(Library)"

rebuild: mrproper all

$(Library): $(Objects)
	echo "* Linking library $(Library)..."
	$(LD) $(LDFLAGS) $^ -o $@

%.uo: %.c
	echo "* Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	echo "* Removing object files..."
	rm *.o || exit 0
	rm -rf $(Objects) || exit 0

mrproper: clean
	echo "* Removing library..."
	rm *.o || exit 0
	rm -rf $(Library) || exit 0
