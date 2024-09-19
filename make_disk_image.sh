#!/bin/bash

NAME=disk.img

dd if=/dev/zero of=${NAME} bs=1M count=100
mkfs.ext2 ${NAME}
