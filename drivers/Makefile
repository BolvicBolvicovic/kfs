DRIVERS	= drivers.a

VGA_SRC	= vga/vga.c
VGA_OBJ	= vga/vga.o

CPU_SRC	= cpu/idt.c cpu/irq.c cpu/isr.c
CPU_OBJ	= cpu/idt.o cpu/irq.o cpu/isr.o

KEY_SRC	= keyboard/keyboard.c
KEY_OBJ	= keyboard/keyboard.o

AR		= ../gcc_kfs/bin/i386-elf-ar rc
CC		= ../gcc_kfs/bin/i386-elf-gcc
FLAGS	= -ffreestanding	\
			-nostdlib		\
			-O2						

all: $(DRIVERS)

$(DRIVERS): $(VGA_OBJ) cpu_asm $(CPU_OBJ) $(KEY_OBJ)
	$(AR) $@ $(VGA_OBJ) $(CPU_OBJ) $(KEY_OBJ)
	
$(VGA_OBJ): $(VGA_SRC)
	$(CC) $(FLAGS) -c $^ -o $@

cpu_asm:
	$(CC) $(FLAGS) -c cpu/isr.s -o cpu/ISR.o

$(CPU_OBJ): $(CPU_SRC)
	$(CC) $(FLAGS) -c $^ -o $@

$(KEY_OBJ): $(KEY_SRC)
	$(CC) $(FLAGS) -c $^ -o $@

clean:
	rm $(VGA_OBJ)
	rm cpu/ISR.o
	rm $(CPU_OBJ)
	rm $(KEY_OBJ)

fclean: clean
	rm $(DRIVERS)

re: fclean all

.PHONY: clean fclean re
