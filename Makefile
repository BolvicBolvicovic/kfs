BINARY		=	isoroot/boot/kfs.elf
ISO			=	isoroot/boot/kfs.iso
CC			=	./gcc_kfs/bin/i386-elf-gcc
LIBS		=	src/string.a src/stdlib.a
OFLAGS		= 	-ffreestanding			\
				-O2						\
				-std=gnu99				
CFLAGS		=	-ffreestanding			\
				-nostdlib				\
				-O2						\
				-T linker/linker.ld	


CSRCS_NAMES	=	start kernel
CSRCS		=	$(addprefix src/, $(addsuffix .c, $(CSRCS_NAMES)))
OBJS		=	$(addprefix obj/, $(addsuffix .o, $(CSRCS_NAMES)))


all			:	 $(ISO)

required	:
	@if [ ! -d obj ]; then mkdir obj; fi
	make -C src

$(ISO)		:	$(BINARY)
	grub-mkrescue isoroot/boot -o $@

$(BINARY)	:	required $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $@ -lgcc

obj/%.o			: src/%.c
	$(CC) $(OFLAGS) -c $^ -o $@

obj/%.o			: src/%.s
	$(CC) $(OFLAGS) -c $^ -o $@

binary		:	$(BINARY)

clean		:
	rm $(OBJS)

fclean		: clean
	make -C src fclean
	rm -d obj
	@if [ -f $(ISO) ];then rm $(ISO); fi
	rm $(BINARY)

re			: fclean all

.PHONY		: all clean fclean re
