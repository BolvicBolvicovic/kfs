KSHELL	= kshell.a

SRC	= kshell.c tester.c 
OBJ	= kshell.o tester.o 

AR		= ../gcc_kfs/bin/i386-elf-ar rcs
CC		= ../gcc_kfs/bin/i386-elf-gcc
FLAGS	= -ffreestanding	\
	  -g \
	  -O2 \
	  -nostdlib		

all: $(KSHELL)

$(KSHELL): $(OBJ)
	$(AR) $@ $(OBJ)
	
%.o: %.c
	$(CC) $(FLAGS) -c $^ -o $@

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(KSHELL)

re: fclean all

.PHONY: clean fclean re
