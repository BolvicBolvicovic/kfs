#include "paging.h"

extern void enable_paging(uint32_t*);

uint32_t page_directory[NUM_PAGES] __attribute__((aligned(PAGE_FRAME_SIZE)));
uint32_t page_table[NUM_PAGES] __attribute__((aligned(PAGE_FRAME_SIZE)));

void initialise_paging() {
    register_interrupt_handler(14, page_table);
    size_t i;
    for (i = 0; i < NUM_PAGES; i++) {
        page_directory[i] = 0x00000002;
    }
    for (i = 0; i < NUM_PAGES; i++) {
        page_table[i] = (i * 0x1000) | 3;
    }
    page_directory[0] = ((uint32_t)page_table) | 3;
    enable_paging(page_directory);
}

void page_fault(registers_t* regs) {
    printf("Page Error");
}
