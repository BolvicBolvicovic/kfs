MEM	= memory.a

PMM_SRC = pmm/pmm.c 
PMM_OBJ = pmm/pmm.o

VMM_SRC = vmm/vmm.c vmm/paging.s vmm/pdt.c vmm/kmalloc.c 
VMM_OBJ = vmm/vmm.o vmm/paging.o vmm/pdt.o vmm/kmalloc.o

AR		= ../gcc_kfs/bin/i386-elf-ar rcs
CC		= ../gcc_kfs/bin/i386-elf-gcc
FLAGS	= -ffreestanding	\
	  -g \
	  -O2 \
	  -Wall \
	  -nostdlib		

all: $(MEM)

$(MEM): $(PMM_OBJ) $(VMM_OBJ)
	$(AR) $@ $^

pmm/%.o: pmm/%.c
	$(CC) $(FLAGS) -c $^ -o $@

vmm/%.o: vmm/%.c
	$(CC) $(FLAGS) -c $^ -o $@

vmm/%.o: vmm/%.s
	$(CC) $(FLAGS) -c $^ -o $@

clean:
	rm -rf $(PMM_OBJ)
	rm -rf $(VMM_OBJ)

fclean: clean
	rm -rf $(MEM)

re: fclean all

.PHONY: clean fclean re
