#!/bin/bash

mkdir -p rootfs/{bin,sbin,etc,proc,sys,dev,tmp} # Create the root directory

dd if=/dev/zero of=./isoroot/boot/initrd.img bs=1M count=4  # Create a 4MB blank image
mkfs.ext2 -F ./isoroot/boot/initrd.img                      # Format it as ext2
sudo mount -o loop ./isoroot/boot/initrd.img /mnt           # Mount the image
sudo cp -r rootfs/* /mnt                     # Copy your root filesystem to the image
sudo umount /mnt                             # Unmount the image
