#ifndef PAGING_H
#define PAGING_H
#include <stddef.h>
#include <stdint.h>
#include "../../drivers/descriptor/descriptor.h"

#define PANIC(x) printf("PANIC(%s) at %s:%s", x, __FILE__, __LINE__); while(1);

uint32_t kmalloc(size_t size, int align, uint32_t* phys_addr);
void init_kmalloc();

typedef struct {
	uint8_t present : 1;
	uint8_t rw 	    : 1;
	uint8_t user 	: 1;
    uint8_t pwt     : 1;
    uint8_t pcd     : 1;
	uint8_t accessed: 1;
	uint8_t dirty 	: 1;
    uint8_t pat     : 1;
    uint8_t g       : 1;
    uint8_t ignored2: 3;
	uint32_t frame 	: 20;
} __attribute__((packed)) page_t;

typedef struct {
	page_t pages[1024];
} page_table_t;

typedef struct {
    uint8_t present : 1;
    uint8_t rx      : 1;
    uint8_t user    : 1;
    uint8_t pwt     : 1;
    uint8_t pcd     : 1;
    uint8_t accessed: 1;
    uint8_t ignored : 1;
    uint8_t ps      : 1;
    uint8_t ignored2: 4;
    uint32_t p_table:20;
} __attribute__((packed)) page_directory_t;

void initialise_paging();
void switch_page_dir(page_directory_t* new_dir);
page_t* get_page(uint32_t addr, int make, page_directory_t* dir);
void page_fault(registers_t* regs);

#endif
