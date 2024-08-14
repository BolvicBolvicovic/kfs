STRING	= string.a
STR_SRC	= string/string.c
STR_OBJ	= string/string.o

STDLIB	= stdlib.a
STD_SRC	= stdlib/stdlib.c
STD_OBJ	= stdlib/stdlib.o

VGA		= vga.a
VGA_SRC	= vga/vga.c
VGA_OBJ	= vga/vga.o

STDIO	= stdio.a
IO_SRC	= stdio/stdio.c
IO_OBJ	= stdio/stdio.o

AR		= ../gcc_kfs/bin/i386-elf-ar rc
CC		= ../gcc_kfs/bin/i386-elf-gcc
FLAGS	= -ffreestanding	\
			-nostdlib		\
			-O2						

all: $(STRING) $(STDLIB) $(VGA) $(STDIO)

$(STDIO):	$(IO_OBJ)
	$(AR) $@ $^ -L$(STDLIB)

$(IO_OBJ): $(IO_SRC) $(STDLIB)
	$(CC) $(FLAGS) -c $^ -o $@

$(VGA):	$(VGA_OBJ)
	$(AR) $@ $^

$(VGA_OBJ): $(VGA_SRC)
	$(CC) $(FLAGS) -c $^ -o $@

$(STDLIB):	$(STD_OBJ)
	$(AR) $@ $^

$(STD_OBJ): $(STD_SRC)
	$(CC) $(FLAGS) -c $^ -o $@

$(STRING):	$(STR_OBJ)
	$(AR) $@ $^

$(STR_OBJ): $(STR_SRC)
	$(CC) $(FLAGS) -c $^ -o $@

clean:
	rm $(STR_OBJ)
	rm $(STD_OBJ)
	rm $(IO_OBJ)
	rm $(VGA_OBJ)

fclean: clean
	rm $(STRING)
	rm $(STDLIB)
	rm $(STDIO)
	rm $(VGA)

re: fclean all

.PHONY: clean fclean re
