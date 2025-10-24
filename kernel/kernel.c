#include "kernel.h"

#if defined(__linux__)
	#error "This code must be compiled with a cross-compiler"
#elif !defined(__i386__)
	#error "This code must be compiled with an x86-elf compiler"
#endif

typedef struct
{
	uint32_t	startLo;	//base address
	uint32_t	startHi;
	uint32_t	sizeLo;		//length (in bytes)
	uint32_t	sizeHi;
	uint32_t	type;
	uint32_t	acpi_3_0;
} memory_region;

char* strMemoryTypes[] =
{
	"Available",		//memory_region.type==1
	"Reserved",			//memory_region.type==2
	"ACPI Reclaim",		//memory_region.type==3
	"ACPI NVS Memory"	//memory_region.type==4
	"Bad RAM"       	//memory_region.type==5
};

extern uint32_t start_kernel;
extern uint32_t endkernel;
extern uint32_t start_kernel_virt;
extern uint32_t end_kernel_virt;
extern uint32_t bitmap;

#define MAX_MEMORY_SIZE (0xFFFFFFFF / 0x1000)

void
kernel_main(uint32_t magic, uint32_t addr)
{
    multiboot_info_t* mbi = (multiboot_info_t*)addr;
    struct multiboot_mmap_entry* region = (struct multiboot_mmap_entry*) mbi->mmap_addr;
    uint32_t region_count = mbi->mmap_length / sizeof(struct multiboot_mmap_entry);
    // uint32_t kernel_size = ((uint32_t)&endkernel - (uint32_t)&start_kernel);
    // uint32_t kernel_size_aligned = (kernel_size + 0x1000 - 1) & ~0xFFF;
	uint32_t	end_kernel_aligned = ((uint32_t)&endkernel + 0xFFF) & ~0xFFF;

    init_kshell(BLUE, WHITE);
    term_clear();
    isr_install();
    init_keyboard();
    init_timer(250);
    init_syscall();
    pmm_init(MAX_MEMORY_SIZE, (uint32_t)&bitmap);
    for (size_t i = 0; i < region_count; i++)
    {
        if (region[i].type > 5) region[i].type = MULTIBOOT_MEMORY_AVAILABLE;
        if (region[i].type != MULTIBOOT_MEMORY_AVAILABLE) pmm_deinit_region(region[i].addr_low, region[i].len_low);
    }
	//pmm_deinit_region((uint32_t)&start_kernel, kernel_size_aligned);
	// Note: As writing in some part of region that is initialized in the first MB creates big errors,
	// we just deinit all of it to be sure that there would not be any problem.
	pmm_deinit_region(0, end_kernel_aligned);
	init_vmm();
	init_gpu();
    //ide_init(0x1F0, 0x3F6, 0x170, 0x376, 0x000);

	init_multitasking();

}
