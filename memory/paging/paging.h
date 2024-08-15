#ifndef PAGING_H
#define PAGING_H

#include <stddef.h>
#include <stdint.h>
#include "../../drivers/descriptor/descriptor.h"

#define PANIC(x) printf("PANIC(%s) at %s:%s", x, __FILE__, __LINE__); while(1);

#define NUM_PAGES 1024
#define PAGE_FRAME_SIZE 4096

#define PRESENT     1
#define PAGE_RO     0
#define PAGE_RW     1
#define PAGE_USER   1
#define PAGE_KERNEL 0
#define PAGE_SZ_4KB 0
#define PAGE_SZ_4MB 1

uint32_t kmalloc(size_t size, int align, uint32_t* phys_addr);
uint32_t kmalloc_page();

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
    uint8_t ignored : 3;
	uint32_t frame 	: 20;
} __attribute__((packed)) page_t;

typedef struct {
	page_t pages[1024] __attribute__((aligned(4096)));
} page_table_t;

typedef struct {
    page_table_t *tables[1024];
    uint32_t    tables_physical[1024];
    uint32_t    physical_addr;
} page_directory_t;

void initialise_paging();
void page_fault(registers_t* regs);

#endif
