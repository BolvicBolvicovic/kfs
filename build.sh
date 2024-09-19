#!/bin/bash

TOOLCHAIN_DIR="i386-elf-7.5.0-Linux-x86_64"

GREEN='\033[0;32m'
YELLOW='\033[0;33m'
WHITE='\033[0m'

print_waiting() {
	local pid=$1
	local delay=0.1
	local spin='|/-\\'
	local i=0
	while kill -0 $pid >/dev/null 2>/dev/null; do
		i=$(( (i + 1) % 4 ))
		printf "\r${YELLOW}Processing...${WHITE} ${spin:$i:1}"
		sleep $delay
	done
	printf "\r${GREEN}Done!             ${WHITE}\n"
        echo -e "${YELLOW}Renaming directory...${WHITE}"
	mv ${TOOLCHAIN_DIR} gcc_kfs && rm ${TOOLCHAIN_DIR}.tar.xz
}


if [[ ! -d gcc_kfs ]]; then
	echo -e "${YELLOW}Downloading toolchain...${WHITE}"
	wget https://newos.org/toolchains/${TOOLCHAIN_DIR}.tar.xz

	echo -e "${YELLOW}Extracting files...${WHITE}"
	tar xf ${TOOLCHAIN_DIR}.tar.xz &
	print_waiting $!
fi
echo -e "${GREEN}Toolchain setup complete.${WHITE}"
echo -e "${YELLOW}Building and launching kernel...${WHITE}"
bash make_disk_image
make qemu
