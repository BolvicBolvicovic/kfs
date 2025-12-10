LIBC	= libc.a

SRCS	= string.c stdlib.c stdio.c
OBJS	= $(SRCS:.c=.o)

AR	= ../gcc_kfs/bin/i386-elf-ar rcs
CC	= ../gcc_kfs/bin/i386-elf-gcc
INC	= -I../include -I..
FLAGS	= -ffreestanding	\
	  -g \
	  -O2 \
	  -nostdlib		

all: $(LIBC)

$(LIBC): $(OBJS)
	$(AR) $@ $^
	
%.o: %.c
	$(CC) $(FLAGS) $(INC) -c $< -o $@

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(LIBC)

re: fclean all

.PHONY: clean fclean re
