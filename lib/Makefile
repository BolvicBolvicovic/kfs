LIBC	= libc.a

STR_SRC	= string/string.c
STR_OBJ	= string/string.o

STD_SRC	= stdlib/stdlib.c
STD_OBJ	= stdlib/stdlib.o

VGA_SRC	= vga/vga.c
VGA_OBJ	= vga/vga.o

IO_SRC	= stdio/stdio.c
IO_OBJ	= stdio/stdio.o

AR		= ../gcc_kfs/bin/i386-elf-ar rc
CC		= ../gcc_kfs/bin/i386-elf-gcc
FLAGS	= -ffreestanding	\
			-nostdlib		\
			-O2						

all: $(LIBC)

$(LIBC): $(STD_OBJ) $(IO_OBJ) $(VGA_OBJ) $(STR_OBJ)
	$(AR) $@ $^
	
$(IO_OBJ): $(IO_SRC)
	$(CC) $(FLAGS) -c $^ -o $@

$(VGA_OBJ): $(VGA_SRC)
	$(CC) $(FLAGS) -c $^ -o $@

$(STD_OBJ): $(STD_SRC)
	$(CC) $(FLAGS) -c $^ -o $@

$(STR_OBJ): $(STR_SRC)
	$(CC) $(FLAGS) -c $^ -o $@

clean:
	rm $(STR_OBJ)
	rm $(STD_OBJ)
	rm $(IO_OBJ)
	rm $(VGA_OBJ)

fclean: clean
	rm $(LIBC)

re: fclean all

.PHONY: clean fclean re
