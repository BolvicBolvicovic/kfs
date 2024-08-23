BINARY		=	isoroot/boot/kfs.elf
ISO			=	kfs.iso
CC			=	./gcc_kfs/bin/i386-elf-gcc
LD			=	./gcc_kfs/bin/i386-elf-ld
LIBS		=	lib/libc.a drivers/drivers.a memory/memory.a
CFLAGS		= 	-ffreestanding			\
				-g	\
				-O2 \
				-std=gnu99				
LFLAGS		=	-T linker/linker.ld --whole-archive

SRCS_DIR	=	kernel/
CSRCS_NAMES	=	start kernel
CSRCS		=	$(addprefix $(SRCS_DIR), $(addsuffix .c, $(CSRCS_NAMES)))
OBJS		=	$(addprefix obj/, $(addsuffix .o, $(CSRCS_NAMES)))


all		:	 $(ISO)

required	:
	@if [ ! -d obj ]; then mkdir obj; fi
	make -C lib
	make -C drivers
	make -C memory

$(ISO)		:	$(BINARY)
	grub-mkrescue -o $@ isoroot

$(BINARY)	:	required $(OBJS)
	$(LD) $(LFLAGS) $(OBJS) $(LIBS) -o $@

obj/%.o		: $(SRCS_DIR)%.c
	$(CC) $(CFLAGS) -c $^ -o $@

obj/%.o		: $(SRCS_DIR)%.s
	$(CC) $(OFLAGS) -c $^ -o $@

qemu		: $(ISO)
	qemu-system-i386 -cdrom $^

qemu_debug	: $(ISO)
	qemu-system-i386 -cdrom $^ -s -S

clean		:
	rm -rf $(OBJS)
	make -C lib clean
	make -C drivers clean

fclean		: clean
	make -C lib fclean
	make -C drivers fclean
	make -C memory fclean
	rm -rf obj
	rm -rf $(ISO)
	rm -rf $(BINARY)

re		: fclean all

.PHONY		: all clean fclean re
