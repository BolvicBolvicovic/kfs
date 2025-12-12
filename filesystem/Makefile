MEM	= filesystem.a

SRCS = fs.c 
OBJS = fs.o

AR	= ../gcc_kfs/bin/i386-elf-ar rcs
CC	= ../gcc_kfs/bin/i386-elf-gcc -I ../drivers
INC	= -I.. -I../include
FLAGS	= -ffreestanding	\
	  -g \
	  -O0 \
	  -Wall \
	  -nostdlib		

all: $(MEM)

$(MEM): $(OBJS)
	$(AR) $@ $^

%.o: %.c
	$(CC) $(FLAGS) $(INC) -c $^ -o $@

clean:
	rm -rf $(VFS_OBJ)
	rm -rf $(IDE_OBJ)

fclean: clean
	rm -rf $(MEM)

re: fclean all

.PHONY: clean fclean re
