FS	= filesystem.a

SRCS	= fs.c fdtable.c path.c socket.c
OBJS	= $(SRCS:.c=.o)

AR	= ../gcc_kfs/bin/i386-elf-ar rcs
CC	= ../gcc_kfs/bin/i386-elf-gcc
INC	= -I.. -I../include
FLAGS	= -ffreestanding\
	  -g		\
	  -O0		\
	  -Wall		\
	  -nostdlib

all: $(FS)

$(FS): $(OBJS)
	$(AR) $@ $^

%.o: %.c
	$(CC) $(FLAGS) $(INC) -c $^ -o $@

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(FS)

re: fclean all

.PHONY: clean fclean re
