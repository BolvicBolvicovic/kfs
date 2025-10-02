PRO	= processes.a

SRCS = switch.s multitasking.c syscalls.c
OBJS = switch.o multitasking.o syscalls.o

AR		= ../gcc_kfs/bin/i386-elf-ar rcs
CC		= ../gcc_kfs/bin/i386-elf-gcc -I. -I../lib
FLAGS	= -ffreestanding	\
	  -g \
	  -O0 \
	  -Wall \
	  -nostdlib

all: $(PRO)

$(PRO): $(OBJS) 
	$(AR) $@ $^

%.o: %.c
	$(CC) $(FLAGS) -c $^ -o $@

%.o: %.s
	$(CC) $(FLAGS) -c $^ -o $@

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(PRO)

re: fclean all

.PHONY: clean fclean re
