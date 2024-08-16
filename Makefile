BINARY		=	isoroot/boot/kfs.elf
ISO			=	kfs.iso
CC			=	./gcc_kfs/bin/i386-elf-gcc
LD			=	./gcc_kfs/bin/i386-elf-ld
LIBS		=	lib/libc.a drivers/drivers.a
CFLAGS		= 	-ffreestanding			\
				-O2						\
				-std=gnu99				
LFLAGS		=	-T linker/linker.ld

SRCS_DIR	=	kernel/
CSRCS_NAMES	=	start kernel
CSRCS		=	$(addprefix $(SRCS_DIR), $(addsuffix .c, $(CSRCS_NAMES)))
OBJS		=	$(addprefix obj/, $(addsuffix .o, $(CSRCS_NAMES)))


all			:	 $(ISO)

required	:
	@if [ ! -d obj ]; then mkdir obj; fi
	make -C drivers
	make -C lib

$(ISO)		:	$(BINARY)
	grub-mkrescue -o $@ isoroot

$(BINARY)	:	required $(OBJS)
	$(LD) $(LFLAGS) $(OBJS) $(LIBS) -o $@

obj/%.o			: $(SRCS_DIR)%.c
	$(CC) $(CFLAGS) -c $^ -o $@

obj/%.o			: $(SRCS_DIR)%.s
	$(CC) $(OFLAGS) -c $^ -o $@

binary		:	$(BINARY)

clean		:
	rm $(OBJS)

fclean		: clean
	make -C lib fclean
	make -C drivers fclean
	rm -d obj
	@if [ -f $(ISO) ];then rm $(ISO); fi
	rm $(BINARY)

re			: fclean all

.PHONY		: all clean fclean re
