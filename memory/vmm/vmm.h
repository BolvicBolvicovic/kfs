#ifndef VMM_H
#define VMM_H

#include "pdt.h"
#include "../pmm/pmm.h"

#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR	1024

#define PAGE_DIR_INDEX(ADDR) (((ADDR) >> 22) & 0x3FF)
#define PAGE_TAB_INDEX(ADDR) (((ADDR) >> 12) & 0x3FF)
#define PAGE_PHYS_ADDR(ADDR) ((*ADDR) & ~0xFFF)

#define PTABLE_ADDR_SPACE_SIZE 0x400000	     // 4MB
#define DTABLE_ADDR_SPACE_SIZE 0x100000000   // 4GB

#define PAGE_SIZE 0x1000		     // 4KB

typedef struct {
   pt_entry m_entries[PAGES_PER_TABLE];
} p_table;

typedef struct {
   pd_entry m_entries[PAGES_PER_DIR];
} p_dir;

void vmm_init();

#endif
