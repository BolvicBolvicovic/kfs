MEM	= memory.a

PAG_SRC	= paging/paging.c paging/kmalloc.c paging/enable_paging.s 
PAG_OBJ	= paging/paging.o paging/kmalloc.o paging/enable_paging.o

AR		= ../gcc_kfs/bin/i386-elf-ar rcs
CC		= ../gcc_kfs/bin/i386-elf-gcc
FLAGS	= -ffreestanding	\
	  -g \
	  -O2 \
	  -nostdlib		

all: $(MEM)

$(MEM): $(PAG_OBJ)
	$(AR) $@ $^
	
paging/%.o: paging/%.c
	$(CC) $(FLAGS) -c $^ -o $@

paging/%.o: paging/%.s
	$(CC) $(FLAGS) -c $^ -o $@

clean:
	rm -rf $(PAG_OBJ)

fclean: clean
	rm -rf $(MEM)

re: fclean all

.PHONY: clean fclean re
