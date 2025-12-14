MEM	= memory.a

PMM_SRC = $(addprefix pmm/, pmm.c) 
PMM_OBJ = $(PMM_SRC:.c=.o)

VMM_SRC = $(addprefix vmm/, vmm.c paging.s)
VMM_OBJ = $(addprefix vmm/, vmm.o paging.o)

ALL_SRC = $(addprefix allocators/, kmalloc.c karena.c)
ALL_OBJ = $(ALL_SRC:.c=.o)

AR	= ../gcc_kfs/bin/i386-elf-ar rcs
CC	= ../gcc_kfs/bin/i386-elf-gcc
INC	= -I../include -I..
FLAGS	= -ffreestanding	\
	  -g \
	  -O2 \
	  -Wall \
	  -nostdlib		

all: $(MEM)

$(MEM): $(PMM_OBJ) $(VMM_OBJ) $(ALL_OBJ)
	$(AR) $@ $^

pmm/%.o: pmm/%.c
	$(CC) $(FLAGS) $(INC) -c $^ -o $@

vmm/%.o: vmm/%.c
	$(CC) $(FLAGS) $(INC) -c $^ -o $@

vmm/%.o: vmm/%.s
	$(CC) $(FLAGS) $(INC) -c $^ -o $@

allocators/%.o: allocators/%.c
	$(CC) $(FLAGS) $(INC) -c $^ -o $@

clean:
	rm -rf $(PMM_OBJ)
	rm -rf $(VMM_OBJ)
	rm -rf $(ALL_OBJ)

fclean: clean
	rm -rf $(MEM)

re: fclean all

.PHONY: clean fclean re
