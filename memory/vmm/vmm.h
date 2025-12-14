#ifndef VMM_H
#define VMM_H

#include <memory/pmm/pmm.h>
#include <bits.h>
#include <sizes.h>

#define PAGES_PER_TABLE			1024
#define TABLES_PER_DIR			1024

#define PAGE_DIR_INDEX(ADDR)		(((ADDR) >> 22) & 0x3FF)
#define PAGE_TAB_INDEX(ADDR)		(((ADDR) >> 12) & 0x3FF)
#define PAGE_PHYS_ADDR(ENTRY)		((ENTRY) & ~0xFFF)

#define PTABLE_ADDR_SPACE_SIZE		MB(4)
#define DTABLE_ADDR_SPACE_SIZE		GB(4)

#define PAGE_SIZE			KB(4)
#define PAGE_LARGE_SIZE			MB(4)

#define PROCESS_CODE_START		0x08048000
#define PROCESS_STACK_START		0xBFFF0000
#define	STACK_PAGES			16		// Note: 64KB
#define KPD_ENTRIES_START		768

#define PE_KERNEL			0
#define PE_PRESENT			BIT0
#define PE_WRITABLE			BIT1
#define PE_USER				BIT2

#define KTABLES_SIZE			(KPD_ENTRIES_END - KPD_ENTRIES_START)
// The page directory is always accessible at this virtual address.
// It's the last page in the virtual address space.
#define RECURSIVE_PAGEDIR_ADDR		0xFFFFF000

// All page tables are accessible as a contiguous 4MB array at this address.
#define RECURSIVE_PAGETABLES_ADDR	0xFFC00000


typedef u32	pt_entry; 
typedef u32	pd_entry; 

// Note: the user parameter in the vmm functions should be set to either I86_PTE_KERNEL (0) or I86_PTE_USER (4)

void	init_vmm(void);
u32	vmm_setup_process(u32 code_size, u32 data_size, u32* code, u32* data);
void	vmm_set_flags_pages(u32 virt_addr, u32 nb_blocks, u32 flags, uint8_t set);
void*	vmm_alloc_kblocks(u32 nb_blocks);
void	vmm_free_blocks(u32 virtual_addr, u32 nb_blocks);
void	vmm_map_kpage(u32 phys, u32 virt);
u32	mmap_user(u32 addr, u32 len, u32 prot, u32 flags, u32 fd, u32 off);
u32	vmm_virt_to_phys(void* virt_addr);
u32	vmm_find_next_frees_kernel(u32 nb_blocks);
void*	vmm_reserve_kblocks(u32 nb_blocks);
u32	vmm_commit_kblocks(u32 virtual_addr, u32 nb_blocks);

#endif
