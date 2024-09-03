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
	"Available",		//memory_region.type==0
	"Reserved",			//memory_region.type==1
	"ACPI Reclaim",		//memory_region.type==2
	"ACPI NVS Memory"	//memory_region.type==3
};
extern uint32_t endkernel;
extern uint32_t code;
void	kernel_main(uint32_t magic, uint32_t addr) {
    
    multiboot_info_t* mbi = (multiboot_info_t*)addr;
    uint32_t kheap_start = (uint32_t)&endkernel + 0x1000;
    uint32_t kheap_end = kheap_start + 0x100000;
    memory_region* region = (memory_region*) 0x1000;
//    uint32_t mem_size = 1024 + mbi->mem_lower + mbi->mem_upper * 64;
    uint32_t mem_size = 130816;

    init_current_screen(YELLOW, BLUE);
	term_clear();
	isr_install();
	init_keyboard();
	init_timer(50);
    pmm_init(mem_size, kheap_start);
    printf("PMM initialized with %d KB\n", mem_size);

    printf("\nPM map\n");
    for (size_t i = 0; i < 15; i++) {
        if (region[i].type > 4)              region[i].type = 1;
        if (i > 0 && region[i].startLo == 0) break;
        printf("region %d: start high 0x%x s_low 0x%x length (bytes): 0x%x l_low 0x%x type: %d (%s)\n", i,
            region[i].startHi, region[i].startLo,
            region[i].sizeHi, region[i].sizeLo,
            region[i].type, strMemoryTypes[region[i].type]
        );
        if (region[i].type == 1) pmm_init_region(region[i].startLo, region[i].sizeLo);
    }
    pmm_deinit_region(0x10000, &endkernel - &code);
	//initialise_paging();
	asm volatile("sti\n\t");
    uint32_t* p = (uint32_t*)pmm_allock_block();
    printf("\np allocated at   0x%x\n", p);
    pmm_free_block(p);
    p = (uint32_t*)pmm_allock_block();
    printf("p reallocated at 0x%x\n", p);
    uint32_t* f = (uint32_t*)pmm_allock_block();
    printf("f allocated at   0x%x\n", p);
}
