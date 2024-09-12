#include "kernel.h"

#if defined(__linux__)
	#error "This code must be compiled with a cross-compiler"
#elif !defined(__i386__)
	#error "This code must be compiled with an x86-elf compiler"
#endif
typedef struct {

	uint32_t	startLo;	//base address
	uint32_t	startHi;
	uint32_t	sizeLo;		//length (in bytes)
	uint32_t	sizeHi;
	uint32_t	type;
	uint32_t	acpi_3_0;
} memory_region;
char* strMemoryTypes[] = {
	"Available",		//memory_region.type==1
	"Reserved",			//memory_region.type==2
	"ACPI Reclaim",		//memory_region.type==3
	"ACPI NVS Memory"	//memory_region.type==4
	"Bad RAM"       	//memory_region.type==5
};
extern uint32_t end_kernel_virt;
extern uint32_t start_kernel_virt;
extern uint32_t bitmap;
#define MAX_MEMORY_SIZE 0xFFFFF


void	kernel_main(uint32_t magic, uint32_t addr) {
    multiboot_info_t* mbi = (multiboot_info_t*)addr;
    struct multiboot_mmap_entry* region = (struct multiboot_mmap_entry*) mbi->mmap_addr;
    uint32_t mem_size = MAX_MEMORY_SIZE;

    init_current_screen(BLUE, WHITE);
    term_clear();
    isr_install();
    init_keyboard();
    init_timer(50);
    init_syscall();
    asm volatile(
	"mov $1, %eax\n"
	"int $0x80"
    );
    pmm_init(mem_size, &bitmap);
    for (size_t i = 0; i < 15; i++) {
        if (region[i].type > 5)           region[i].type = MULTIBOOT_MEMORY_AVAILABLE;
        if (i > 0 && region[i].addr_low == 0) break;
        printf ("region %d: start: %p length (bytes): %p type: %d (%s)\n", i, 
			region[i].addr_low,
			region[i].len_low,
			region[i].type, strMemoryTypes[region[i].type-1]);
        if (region[i].type == MULTIBOOT_MEMORY_AVAILABLE) pmm_init_region(region[i].addr_low, region[i].len_low);
    }
    pmm_deinit_region(0x100000, 0);
    pmm_deinit_region(0x100000, 0xC0000000);
    vmm_init();
    asm volatile("sti\n\t");
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r" (cr0));
    if (cr0 & 0x80000000) printf("Paging enabled: cr0 == %p\n", cr0);
    else printf("Paging disabled: cr0 == %p\n", cr0);
    uint32_t* test = kmalloc(1);
    printf("test: %p sizeof(test) == %d bytes and value %d\n", test, kget_size(test), *test);
    kfree(test);
    void* test2 = kmalloc(1);
    *test = 5;
    void* test3 = kmalloc(0x1001);
    printf("test %p | test2 %p | test3 %p\n", test, test2, test3);
    printf("sizeof(test) == %d bytes and value %d\n", kget_size(test), *test);
}
