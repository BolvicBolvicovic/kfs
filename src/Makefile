STRING	= string.a
STR_SRC	= string/string.c
STR_OBJ	= string/string.o

STDLIB	= stdlib.a
STD_SRC	= stdlib/stdlib.c
STD_OBJ	= stdlib/stdlib.o

AR		= ../gcc_kfs/bin/i386-elf-ar rc
CC		= ../gcc_kfs/bin/i386-elf-gcc
FLAGS	= -ffreestanding	\
			-nostdlib		\
			-O2						

all: $(STRING) $(STDLIB)

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

fclean: clean
	rm $(STRING)
	rm $(STDLIB)

re: fclean all

.PHONY: clean fclean re
