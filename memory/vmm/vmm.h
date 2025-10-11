#ifndef VMM_H
#define VMM_H

#include "../pmm/pmm.h"

#define PAGES_PER_TABLE 1024
#define TABLES_PER_DIR	1024

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

#define PE_KERNEL	0

#define PE_PRESENT	1

#define PE_WRITABLE	2

#define PE_USER		4

#define KTABLES_SIZE (KPD_ENTRIES_END - KPD_ENTRIES_START)
// The page directory is always accessible at this virtual address.
// It's the last page in the virtual address space.
#define RECURSIVE_PAGEDIR_ADDR 0xFFFFF000

// All page tables are accessible as a contiguous 4MB array at this address.
#define RECURSIVE_PAGETABLES_ADDR 0xFFC00000


typedef uint32_t pt_entry; 
typedef uint32_t pd_entry; 

// Note: the user parameter in the vmm functions should be set to either I86_PTE_KERNEL (0) or I86_PTE_USER (4)

void		init_vmm(void);
pd_entry*	vmm_setup_process(uint32_t code_size, uint32_t data_size, uint32_t* code, uint32_t* data);
void		vmm_set_flags_pages(uint32_t virt_addr, uint32_t nb_blocks, uint32_t flags, uint8_t set);
void*   	vmm_alloc_blocks(size_t size, uint32_t user);
void		vmm_free_blocks(uint32_t virtual_addr, uint32_t nb_blocks);
void*   	kmalloc(size_t size);
void    	kfree(void* virt_addr);
uint32_t	kget_size(void* virt_addr);

#endif
