#!/bin/sh

# We assume mnt/ is the directory where the image is mounted, and src/ is the directory with all the compiled files

# Update GRUB's menu.cfg
cp menu_fdd.cfg mnt/boot/menu.cfg

# copy kernel
cp src/kernel/kernel.elf mnt
cp src/modules/test/test.elf mnt

#echo "*** Launching a BASH shell, if you want to do any maintenance ***"
#bash || exit 0
