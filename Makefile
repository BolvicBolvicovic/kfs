BINARY		=	boot/kfs.elf
ISO			=	boot/kfs.iso
CC			=	./gcc_kfs/cross/bin/i386-elf-gcc
OFLAGS		= 	-ffreestanding			\
				-std=gnu99				
CFLAGS		=	-ffreestanding			\
				-nostdlib				\
				-T linker/linker.ld

CSRCS_NAMES	=	start kernel
CSRCS		=	$(addprefix src/, $(addsuffix .c, $(CSRCS_NAMES)))
OBJS		=	$(addprefix obj/, $(addsuffix .o, $(CSRCS_NAMES)))


all			:	 $(ISO)

required	:
	@if [ ! -d obj ]; then mkdir obj; fi

$(ISO)		:	$(BINARY)
	grub-mkrescue boot -o $@

$(BINARY)	:	required $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ -lgcc

obj/%.o			: src/%.c
	$(CC) $(OFLAGS) -c $^ -o $@

obj/%.o			: src/%.s
	$(CC) $(OFLAGS) -c $^ -o $@

binary		:	$(BINARY)

clean		:
	rm $(OBJS)

fclean		: clean
	rm -d obj
	@if [ -f $(ISO) ];then rm $(ISO); fi
	rm $(BINARY)

re			: fclean all

.PHONY		: all clean fclean re
