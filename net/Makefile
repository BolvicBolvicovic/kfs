NET	= net.a

SRCS	= protocol_unix.c
OBJS	= $(SRCS:.c=.o)

AR	= ../gcc_kfs/bin/i386-elf-ar rcs
CC	= ../gcc_kfs/bin/i386-elf-gcc -I ../drivers
INC	= -I.. -I../include
FLAGS	= -ffreestanding	\
	  -g \
	  -O0 \
	  -Wall \
	  -nostdlib		

all: $(NET)

$(NET): $(OBJS)
	$(AR) $@ $^

%.o: %.c
	$(CC) $(FLAGS) $(INC) -c $^ -o $@

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NET)

re: fclean all

.PHONY: clean fclean re
