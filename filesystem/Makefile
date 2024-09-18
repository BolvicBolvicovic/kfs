MEM	= filesystem.a

VFS_SRC = vfs/vfs.c 
VFS_OBJ = vfs/vfs.o

#IRD_SRC = initrd/initrd.c 
#IRD_OBJ = initrd/initrd.o

AR		= ../gcc_kfs/bin/i386-elf-ar rcs
CC		= ../gcc_kfs/bin/i386-elf-gcc
FLAGS	= -ffreestanding	\
	  -g \
	  -O0 \
	  -Wall \
	  -nostdlib		

all: $(MEM)

$(MEM): $(VFS_OBJ) #$(IRD_OBJ)
	$(AR) $@ $^

vfs/%.o: vfs/%.c
	$(CC) $(FLAGS) -c $^ -o $@

initrd/%.o: initrd/%.c
	$(CC) $(FLAGS) -c $^ -o $@

clean:
	rm -rf $(VFS_OBJ)
	rm -rf $(IRD_OBJ)

fclean: clean
	rm -rf $(MEM)

re: fclean all

.PHONY: clean fclean re
