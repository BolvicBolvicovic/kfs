#include "kernel.h"

#if defined(__linux__)
	#error "This code must be compiled with a cross-compiler"
#elif !defined(__i386__)
	#error "This code must be compiled with an x86-elf compiler"
#endif

typedef struct multiboot_mmap_entry multiboot_mmap_entry;

char* strMemoryTypes[] =
{
	"Available",		// memory_region.type==1
	"Reserved",		// memory_region.type==2
	"ACPI Reclaim",		// memory_region.type==3
	"ACPI NVS Memory"	// memory_region.type==4
	"Bad RAM"       	// memory_region.type==5
};

extern u32 start_kernel;
extern u32 endkernel;
extern u32 start_kernel_virt;
extern u32 end_kernel_virt;
extern u32 bitmap;

#define MAX_MEMORY_SIZE (0xFFFFFFFF / 0x1000)

void
kernel_main(u32 magic, u32 addr)
{
	multiboot_info_t*	mbi = (multiboot_info_t*)addr;
	multiboot_mmap_entry*	region = (multiboot_mmap_entry*) mbi->mmap_addr;
	u32			region_count = mbi->mmap_length / sizeof(multiboot_mmap_entry);
	u32			end_kernel_aligned = ((u32)&endkernel + 0xFFF) & ~0xFFF;
	
	init_kshell(BLUE, WHITE);
	term_clear();
	isr_install();
	init_keyboard();
	init_timer(250);
	init_syscall();
	pmm_init(MAX_MEMORY_SIZE, (u32)&bitmap);
	for (u32 i = 0; i < region_count; i++)
	{
		if (region[i].type > 5) region[i].type = MULTIBOOT_MEMORY_AVAILABLE;
		if (region[i].type != MULTIBOOT_MEMORY_AVAILABLE) pmm_deinit_region(region[i].addr_low, region[i].len_low);
	}
	// Note: As writing in some part of region that is initialized in the first MB creates big errors,
	// we just deinit all of it to be sure that there would not be any problem.
	pmm_deinit_region(0, end_kernel_aligned);
	init_vmm();
	//init_gpu();
	//ide_init(0x1F0, 0x3F6, 0x170, 0x376, 0x000);

	karena_t*	kernel_arena = KARENA_ALLOC();

	fd_init(kernel_arena);
	fs_init();

	init_multitasking();
}
