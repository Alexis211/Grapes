.PHONY: clean, mrproper, Init.rfs, floppy, commit

Projects = stem grapes/test

Kernel = src/stem/stem.elf
Floppy = Grapes.fl.img

all:
	for p in $(Projects); do \
		echo "=> Building $$p"; \
		make -C src/$$p -s; \
	done

rebuild: mrproper all

clean:
	for p in $(Projects); do \
		echo "=> Building $$p"; \
		make -C src/$$p clean -s; \
	done

mrproper:
	for p in $(Projects); do \
		echo "=> Building $$p"; \
		make -C src/$$p mrproper -s; \
	done

commit: mrproper
	git add .
	git commit -a; exit 0
	git push github

floppy:
	mkdir mnt; exit 0
	sudo mount $(Floppy) mnt -o loop
	sudo ./copy_fdd.sh
	sleep 0.3
	sudo umount mnt


bochs: all floppy
	bochs -f bochs.cfg

qemu: all floppy
	qemu -fda $(Floppy) -m 8
