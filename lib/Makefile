LIBC	= libc.a

STR_SRC	= string/string.c
STR_OBJ	= string/string.o

STD_SRC	= stdlib/stdlib.c
STD_OBJ	= stdlib/stdlib.o

IO_SRC	= stdio/stdio.c
IO_OBJ	= stdio/stdio.o

AR	= ../gcc_kfs/bin/i386-elf-ar rcs
CC	= ../gcc_kfs/bin/i386-elf-gcc
INC	= -I../include -I..
FLAGS	= -ffreestanding	\
	  -g \
	  -O2 \
	  -nostdlib		

all: $(LIBC)

$(LIBC): $(STD_OBJ) $(IO_OBJ) $(STR_OBJ)
	$(AR) $@ $^
	
$(IO_OBJ): $(IO_SRC)
	$(CC) $(FLAGS) $(INC) -c $^ -o $@

$(STD_OBJ): $(STD_SRC)
	$(CC) $(FLAGS) $(INC) -c $^ -o $@

$(STR_OBJ): $(STR_SRC)
	$(CC) $(FLAGS) $(INC) -c $^ -o $@

clean:
	rm -rf $(STR_OBJ)
	rm -rf $(STD_OBJ)
	rm -rf $(IO_OBJ)

fclean: clean
	rm -rf $(LIBC)

re: fclean all

.PHONY: clean fclean re
