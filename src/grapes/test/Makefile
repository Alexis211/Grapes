.PHONY: clean, mrproper

CC = gcc
CFLAGS = -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -fno-stack-protector -Wall -Wextra

LD = ld
LDFLAGS = -T ../link.ld

Objects = main.o
Outfile = test

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
	rm $(Objects)

mrproper: clean
	rm $(Outfile)

