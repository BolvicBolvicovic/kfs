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

#define PROCESS_CODE_START	0x08048000
#define PROCESS_STACK_START	0xBFFF0000
#define	STACK_PAGES			16			// Note: 64KB
#define KPD_ENTRIES_START	768
#define KPD_ENTRIES_END		1024


typedef struct
{
   pt_entry m_entries[PAGES_PER_TABLE];
} p_table;

typedef struct
{
   pd_entry m_entries[PAGES_PER_DIR];
} p_dir;

// Note: the user parameter in the vmm functions should be set to either I86_PTE_KERNEL (0) or I86_PTE_USER (4)

p_dir*		vmm_setup_process(uint32_t code_size, uint32_t data_size, uint32_t* code, uint32_t* data);
p_dir*		vmm_get_dir(void);
void		vmm_set_pdir(p_dir* dir);
void		vmm_switch_pdir(p_dir* dir);
void    	vmm_init(void);
void		vmm_set_flags_pages(uint32_t virt_addr, uint32_t nb_blocks, uint32_t flags, uint8_t set);
void*   	vmm_alloc_blocks(size_t size, uint32_t user);
void    	vmm_free_blocks(uint32_t virtual_addr, uint32_t nb_blocks, uint32_t user);
void*   	kmalloc(size_t size);
void    	kfree(void* virt_addr);
uint32_t	kget_size(void* virt_addr);

#endif
