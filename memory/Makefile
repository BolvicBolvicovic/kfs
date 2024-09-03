MEM	= memory.a

PAG_SRC	= paging/paging.c paging/kmalloc.c paging/enable_paging.s 
PAG_OBJ	= paging/paging.o paging/kmalloc.o paging/enable_paging.o

PMM_SRC = pmm/pmm.c 
PMM_OBJ = pmm/pmm.o

AR		= ../gcc_kfs/bin/i386-elf-ar rcs
CC		= ../gcc_kfs/bin/i386-elf-gcc
FLAGS	= -ffreestanding	\
	  -g \
	  -O2 \
	  -nostdlib		

all: $(MEM)

$(MEM): $(PMM_OBJ) #$(PAG_OBJ)
	$(AR) $@ $^

pmm/%.o: pmm/%.c
	$(CC) $(FLAGS) -c $^ -o $@
	
paging/%.o: paging/%.c
	$(CC) $(FLAGS) -c $^ -o $@

paging/%.o: paging/%.s
	$(CC) $(FLAGS) -c $^ -o $@

clean:
	rm -rf $(PAG_OBJ)
	rm -rf $(PMM_OBJ)

fclean: clean
	rm -rf $(MEM)

re: fclean all

.PHONY: clean fclean re
