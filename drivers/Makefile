DRIVERS	= drivers.a

VGA_SRC	= vga/vga.c
VGA_OBJ	= vga/vga.o

#GPU_SRC = gpu/gpu.c
#GPU_OBJ = gpu/gpu.o

PIT_SRC	= pit/pit.c
PIT_OBJ	= pit/pit.o

SYS_SRC	= syscall/syscall.c
SYS_OBJ	= syscall/syscall.o

DES_SRC	= descriptor/idt.c descriptor/irq.c descriptor/isr.c descriptor/interrupt_handlers.s descriptor/gdt.c 
DES_OBJ	= descriptor/idt.o descriptor/irq.o descriptor/isr.o descriptor/interrupt_handlers.o descriptor/gdt.o

KEY_SRC	= keyboard/keyboard.c keyboard/draw_lib.c  
KEY_OBJ	= keyboard/keyboard.o keyboard/draw_lib.o

AR	= ../gcc_kfs/bin/i386-elf-ar rcs
CC	= ../gcc_kfs/bin/i386-elf-gcc
INC	= -I../include -I..
FLAGS	= -ffreestanding	\
	  -g \
	  -O2 \
	  -nostdlib		

all: $(DRIVERS)

$(DRIVERS): $(VGA_OBJ) $(DES_OBJ) $(KEY_OBJ) $(PIT_OBJ) $(SYS_OBJ) #$(GPU_OBJ) 
	$(AR) $@ $(VGA_OBJ) $(DES_OBJ) $(KEY_OBJ) $(PIT_OBJ) $(SYS_OBJ) #$(GPU_OBJ) 
	
$(VGA_OBJ): $(VGA_SRC)
	$(CC) $(FLAGS) $(INC) -c $^ -o $@

#$(GPU_OBJ): $(GPU_SRC)
#	$(CC) $(FLAGS) $(INC) -c $^ -o $@

$(SYS_OBJ): $(SYS_SRC)
	$(CC) $(FLAGS) $(INC) -c $^ -o $@

$(PIT_OBJ): $(PIT_SRC)
	$(CC) $(FLAGS) $(INC) -c $^ -o $@

keyboard/%.o: keyboard/%.c
	$(CC) $(FLAGS) $(INC) -c $^ -o $@

descriptor/%.o: descriptor/%.c
	$(CC) $(FLAGS) $(INC) -c $^ -o $@

descriptor/%.o: descriptor/%.s
	$(CC) $(FLAGS) $(INC) -c $^ -o $@

clean:
	rm -rf $(VGA_OBJ)
#	rm -rf $(GPU_OBJ)
	rm -rf $(SYS_OBJ)
	rm -rf $(DES_OBJ)
	rm -rf $(KEY_OBJ)
	rm -rf $(PIT_OBJ)

fclean: clean
	rm -rf $(DRIVERS)

re: fclean all

.PHONY: clean fclean re
