DRIVERS	= drivers.a

VGA_SRC	= vga/vga.c
VGA_OBJ	= vga/vga.o

DES_SRC	= descriptor/idt.c descriptor/irq.c descriptor/isr.c descriptor/interrupt_handlers.s 
DES_OBJ	= descriptor/idt.o descriptor/irq.o descriptor/isr.o descriptor/interrupt_handlers.o

KEY_SRC	= keyboard/keyboard.c
KEY_OBJ	= keyboard/keyboard.o

AR		= ../gcc_kfs/bin/i386-elf-ar rcs
CC		= ../gcc_kfs/bin/i386-elf-gcc
FLAGS	= -ffreestanding	\
	  -g \
			-nostdlib		\
			-O2						

all: $(DRIVERS)

$(DRIVERS): $(VGA_OBJ) $(DES_OBJ) $(KEY_OBJ)
	$(AR) $@ $(VGA_OBJ) $(DES_OBJ) $(KEY_OBJ)
	
$(VGA_OBJ): $(VGA_SRC)
	$(CC) $(FLAGS) -c $^ -o $@

descriptor/%.o: descriptor/%.c
	$(CC) $(FLAGS) -c $^ -o $@

descriptor/%.o: descriptor/%.s
	$(CC) $(FLAGS) -c $^ -o $@

$(KEY_OBJ): $(KEY_SRC)
	$(CC) $(FLAGS) -c $^ -o $@

clean:
	rm -rf $(VGA_OBJ)
	rm -rf $(DES_OBJ)
	rm -rf $(KEY_OBJ)

fclean: clean
	rm -rf $(DRIVERS)

re: fclean all

.PHONY: clean fclean re
