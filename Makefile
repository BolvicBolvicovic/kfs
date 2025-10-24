BINARY		=	isoroot/boot/kfs.elf
ISO			=	kfs.iso
CC			=	./gcc_kfs/bin/i386-elf-gcc
LD			=	./gcc_kfs/bin/i386-elf-ld
LIBS		=	lib/libc.a memory/memory.a drivers/drivers.a kshell/kshell.a processes/processes.a #filesystem/filesystem.a
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
	make -C memory
	make -C drivers
	make -C kshell
	make -C processes
	#make -C filesystem

$(ISO)		:	$(BINARY)
	grub-mkrescue -o $@ isoroot

$(BINARY)	:	required $(OBJS)
	$(LD) $(LFLAGS) $(OBJS) $(LIBS) -o $@

obj/%.o		: $(SRCS_DIR)%.c
	$(CC) $(CFLAGS) -c $^ -o $@

obj/%.o		: $(SRCS_DIR)%.s
	$(CC) $(CFLAGS) -c $^ -o $@

qemu		: $(ISO)
	qemu-system-i386 \
		-cdrom $< \
		-device virtio-gpu-gl,max_outputs=1,xres=1920,yres=1080 \
		-display gtk,gl=on \
		#-drive file=disk.img,if=ide,format=raw

qemu_logs	: $(ISO)
	qemu-system-i386 \
		-cdrom $< \
		-device virtio-gpu-pci,max_outputs=1,xres=1920,yres=1080 \
		-display gtk,gl=on \
		-d int,cpu_reset \
		-no-reboot
		#-drive file=disk.img,if=ide,format=raw

qemu_debug	: $(ISO)
	qemu-system-i386 \
		-cdrom $< \
		-device virtio-gpu-pci,max_outputs=1,xres=1920,yres=1080 \
		-display gtk,gl=on \
		-s -S
		#-drive file=disk.img,if=ide,format=raw

clean		:
	rm -rf $(OBJS)
	make -C lib clean
	make -C memory clean
	make -C drivers clean
	make -C kshell clean
	make -C processes clean
	#make -C filesystem clean

fclean		: clean
	make -C lib fclean
	make -C drivers fclean
	make -C memory fclean
	#make -C filesystem fclean
	make -C kshell fclean
	make -C processes fclean
	rm -rf obj
	rm -rf $(ISO)
	rm -rf $(BINARY)

re		: fclean all

.PHONY		: all clean fclean re
