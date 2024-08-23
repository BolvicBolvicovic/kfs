#ifndef PAGING_H
#define PAGING_H
#include <stddef.h>
#include <stdint.h>
#include "../../drivers/descriptor/descriptor.h"

#define PANIC(x) printf("File: %s Line: %s Error: %s", __FILE__, __LINE__, x); while(1);

uint32_t kmalloc(size_t size, int align, size_t* phys_addr);
void init_kmalloc();

typedef struct {
	uint32_t present : 1;
	uint32_t rw 	 : 1;
	uint32_t user 	 : 1;
	uint32_t accessed: 1;
	uint32_t dirty 	 : 1;
	uint32_t unused	 : 7;
	uint32_t frame 	 : 20;
} page_t;

typedef struct {
	page_t pages[1024];
} page_table_t;

typedef struct {
	page_table_t* 	tables[1024];
	uint32_t 	physical_tables[1024];
	uint32_t 	phys_addr;
} page_directory_t;

void initialise_paging();
void switch_page_dir(page_directory_t* new_dir);
page_t* get_page(uint32_t addr, int make, page_directory_t* dir);
void page_fault(registers_t* regs);

#endif
