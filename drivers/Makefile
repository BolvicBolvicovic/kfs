DRIVERS	= drivers.a

VGA_SRC	= vga/vga.c
VGA_OBJ	= vga/vga.o

AR		= ../gcc_kfs/bin/i386-elf-ar rc
CC		= ../gcc_kfs/bin/i386-elf-gcc
FLAGS	= -ffreestanding	\
			-nostdlib		\
			-O2						

all: $(DRIVERS)

$(DRIVERS): $(VGA_OBJ)
	$(AR) $@ $^
	
$(VGA_OBJ): $(VGA_SRC)
	$(CC) $(FLAGS) -c $^ -o $@

clean:
	rm $(VGA_OBJ)

fclean: clean
	rm $(DRIVERS)

re: fclean all

.PHONY: clean fclean re
