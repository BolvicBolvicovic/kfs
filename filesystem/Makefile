MEM	= filesystem.a

VFS_SRC = vfs/vfs.c 
VFS_OBJ = vfs/vfs.o

IDE_SRC = ide/ide.c 
IDE_OBJ = ide/ide.o

AR		= ../gcc_kfs/bin/i386-elf-ar rcs
CC		= ../gcc_kfs/bin/i386-elf-gcc -I ../drivers
FLAGS	= -ffreestanding	\
	  -g \
	  -O0 \
	  -Wall \
	  -nostdlib		

all: $(MEM)

$(MEM): $(VFS_OBJ) $(IDE_OBJ)
	$(AR) $@ $^

vfs/%.o: vfs/%.c
	$(CC) $(FLAGS) -c $^ -o $@

ide/%.o: ide/%.c
	$(CC) $(FLAGS) -c $^ -o $@

clean:
	rm -rf $(VFS_OBJ)
	rm -rf $(IDE_OBJ)

fclean: clean
	rm -rf $(MEM)

re: fclean all

.PHONY: clean fclean re
