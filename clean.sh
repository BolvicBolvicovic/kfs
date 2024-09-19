#!/bin/bash

GREEN='\033[0;32m'
YELLOW='\033[0;33m'
WHITE='\033[0m'

echo -e "${YELLOW}Cleaning...${WHITE}"
rm -rf gcc_kfs
make fclean
rm -rf disk.img
echo -e "${GREEN}Done!${WHITE}"
