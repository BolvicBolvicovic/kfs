#include "vmm.h"
#include <stdio.h>
#include <processes/locks.h>

extern void	switch_dir(u32 dir);
extern void	flush_tlb_entry(u32 addr);

#define PAGE_DIR 	((u32*)RECURSIVE_PAGEDIR_ADDR)
#define PAGE_TABLES	((u32(*)[1024])RECURSIVE_PAGETABLES_ADDR)

// TODO: check if a mutex would be better here
static SPINLOCK_DEFINE(sl_kernel);

void
init_vmm(void)
{
	// Note: Alloc all kernel page tables.
	// We sub 2 because the first and the last are already allocated.
	pd_entry	kphys_tables = pmm_alloc_blocks(TABLES_PER_DIR - KPD_ENTRIES_START - 2);

	// Note: Map them.
	for (u32 i = KPD_ENTRIES_START + 1; i < TABLES_PER_DIR - 1; i++, kphys_tables += PAGE_SIZE)
	{
		PAGE_DIR[i] = kphys_tables | PE_PRESENT | PE_WRITABLE;
	}

	// Note: reload page dir
	switch_dir(PAGE_DIR[1023] & ~0xFFF);
}

inline void
vmm_map_kpage(u32 phys, u32 virt)
{
	u32	pd_index = PAGE_DIR_INDEX(virt);
	u32	pt_index = PAGE_TAB_INDEX(virt);

	PAGE_TABLES[pd_index][pt_index] = phys | PE_PRESENT | PE_WRITABLE;
	flush_tlb_entry((u32)virt);
}

static inline void
vmm_unmap_pages(u32 virt, u32 total_virt_blocks)
{
	u32	pd_index = PAGE_DIR_INDEX(virt);
	u32	pt_index = PAGE_TAB_INDEX(virt);

	for (u32 i = 0; i < total_virt_blocks; i++)
	{
		PAGE_TABLES[pd_index + i / PAGES_PER_TABLE][pt_index + i % PAGES_PER_TABLE] = 0;
		flush_tlb_entry(virt + i * PAGE_SIZE);
	}
}

static inline u32
build_virt_addr(u32 pd_index, u32 pt_index)
{
	return (pd_index << 22) | (pt_index << 12);
}

static u32
vmm_find_next_free_user(void)
{
	static u32	user_dir_index = 0;
	static u32	user_tab_index = 0;

	u32	i = user_dir_index;
	u32	j = user_tab_index;

	for (; i < KPD_ENTRIES_START; i++)
	{
		if (PAGE_DIR[i] & PE_PRESENT)
		{
			for (; j < PAGES_PER_TABLE; j++)
			{
				if (!(PAGE_TABLES[i][j] & PE_PRESENT))
				{
					user_dir_index = i; 
					user_tab_index = j + 1;
					return build_virt_addr(i, j);
				}
			}

			j = 0;
		}
		else
		{
			PAGE_DIR[i] = pmm_alloc_block() | PE_PRESENT | PE_WRITABLE | PE_USER;

			if (PAGE_DIR[i] == (PE_PRESENT | PE_WRITABLE | PE_USER))
			{
				PAGE_DIR[i] = 0;
				return 0;
			}

			user_dir_index = i;
			user_tab_index = 1;

			return build_virt_addr(i, 0);
		}
	}

	if (user_dir_index && user_tab_index)
	{
		user_dir_index = 0;
		user_tab_index = 0;

		return vmm_find_next_free_user();
	}

	return 0;
}

static u32
vmm_find_next_free_kernel(void)
{
	static u32	kernel_dir_index = KPD_ENTRIES_START;
	static u32	kernel_tab_index = 0;

	u32	i = kernel_dir_index;
	u32	j = kernel_tab_index;

	for (; i < TABLES_PER_DIR; i++)
	{
		for (; j < PAGES_PER_TABLE; j++)
	    	{
			if (!(PAGE_TABLES[i][j] & PE_PRESENT))
	    		{
	    			kernel_dir_index = i;
	    			kernel_tab_index = j + 1;

	    			return build_virt_addr(i, j);
	    		}
		}

	    	j = 0;
	}
	
	if (kernel_dir_index != KPD_ENTRIES_START && kernel_tab_index)
	{
		kernel_dir_index = KPD_ENTRIES_START;
		kernel_tab_index = 0;

		return vmm_find_next_free_kernel();
	}
	
	return 0;
}

static u32
vmm_find_next_frees_user(u32 nb_blocks)
{
	static u32	user_dir_index = 0;
	static u32	user_tab_index = 0;

	if (nb_blocks == 1) return vmm_find_next_free_user();
	if (nb_blocks > PAGES_PER_TABLE) return 0;

	u32	i = user_dir_index;
	u32	j = user_tab_index;

	for (; i < KPD_ENTRIES_START; i++)
	{
		if (PAGE_DIR[i] & PE_PRESENT)
		{
			for (;j < PAGES_PER_TABLE; j++)
			{
				if (PAGE_TABLES[i][j] & PE_PRESENT) continue;
				
				for (u32 k = 1; k < PAGES_PER_TABLE - j; k++)
				{
					if (!(PAGE_TABLES[i][j + k] & PE_PRESENT) && k + 1 == nb_blocks)
					{
						user_dir_index = i;
						user_tab_index = j + k + 1;
						return build_virt_addr(i, j);
					}
					else if (PAGE_TABLES[i][j + k] & PE_PRESENT) break;
				}
			}

			j = 0;
		}
		else
		{
			PAGE_DIR[i] = pmm_alloc_block() | PE_PRESENT | PE_WRITABLE | PE_USER;

			if (PAGE_DIR[i] == (PE_PRESENT | PE_WRITABLE | PE_USER))
			{
				PAGE_DIR[i] = 0;

				return 0;
			}

			user_dir_index = i;
			user_tab_index = 1;

			return build_virt_addr(i, 0);
		}
	}

	if (user_dir_index && user_tab_index)
	{
		user_dir_index = 0;
		user_tab_index = 0;

		return vmm_find_next_frees_user(nb_blocks);
	}

	return 0;
}

u32
vmm_find_next_frees_kernel(u32 nb_blocks)
{
	static u32	kernel_dir_index = KPD_ENTRIES_START;
	static u32	kernel_tab_index = 0;

	if (nb_blocks > PAGES_PER_TABLE) return 0;

	if (nb_blocks == 1)
	{
		u32	next = vmm_find_next_free_kernel();

		return next;
	}

	u32	i = kernel_dir_index;
	u32	j = kernel_tab_index;

	for (; i < TABLES_PER_DIR; i++)
	{
		for (;j < PAGES_PER_TABLE; j++)
		{
			if (PAGE_TABLES[i][j] & PE_PRESENT) continue;
			
			for (u32 k = 1; k < PAGES_PER_TABLE - j; k++)
			{
				if (!(PAGE_TABLES[i][j + k] & PE_PRESENT) && k + 1 == nb_blocks)
				{
					kernel_dir_index = i;
					kernel_tab_index = j + k + 1;

					return build_virt_addr(i, j);
				}
				else if (PAGE_TABLES[i][j + k] & PE_PRESENT) break;
			}
		}

		j = 0;
	}

	if (kernel_dir_index != KPD_ENTRIES_START && kernel_tab_index)
	{
		kernel_dir_index = 0;
		kernel_tab_index = 0;

		return vmm_find_next_frees_kernel(nb_blocks);
	}

	return 0;
}

// Note: Lazy alloc for user. No need for a alloc user block functions.
// Just call vmm_find_next_frees_user.
void*
vmm_alloc_kblocks(u32 nb_blocks)
{
	spinlock_lock(&sl_kernel);

	u32	virtual_addr = (u32)vmm_find_next_frees_kernel(nb_blocks);

	if (virtual_addr == 0)
	{
		spinlock_unlock(&sl_kernel);
		return 0;
	}

	u32	physical_addr = pmm_alloc_blocks(nb_blocks);

	if (!physical_addr)
	{
		spinlock_unlock(&sl_kernel);
		return 0;
	}

	for (u32 i = 0; i < nb_blocks; i++)
	{
		vmm_map_kpage(physical_addr + (i * PAGE_SIZE), virtual_addr + (i * PAGE_SIZE));
	}
	
	spinlock_unlock(&sl_kernel);

	return (void*)virtual_addr;
}

void
vmm_free_blocks(u32 virtual_addr, u32 nb_blocks)
{
	u32	pd_index = PAGE_DIR_INDEX(virtual_addr);
	u32	pt_index = PAGE_TAB_INDEX(virtual_addr);

	if (virtual_addr < 0x100000 || virtual_addr + nb_blocks*PAGE_SIZE > 0xC0000000)
		spinlock_lock(&sl_kernel);

	pmm_free_blocks(PAGE_TABLES[pd_index][pt_index] & ~0xFFF, nb_blocks);
	vmm_unmap_pages(virtual_addr, nb_blocks);

	if (virtual_addr < 0x100000 || virtual_addr + nb_blocks*PAGE_SIZE > 0xC0000000)
		spinlock_unlock(&sl_kernel);
}

void
vmm_set_flags_pages(u32 virt_addr, u32 nb_blocks, u32 flags, u8 set)
{
	u32		pd_index	= PAGE_DIR_INDEX(virt_addr);
	u32		pt_index	= PAGE_TAB_INDEX(virt_addr);
	pt_entry*	table		= &PAGE_TABLES[pd_index][pt_index];

	if (set)
	{
		for (u32 i = 0; i < nb_blocks; i++)
		{
			table[i] |= flags;
		}
	}
	else
	{
		for (u32 i = 0; i < nb_blocks; i++)
		{
			table[i] &= ~flags;
		}
	}
}

u32
vmm_setup_process(u32 code_size, u32 data_size, u32* code, u32* data)
{
	u32	i = 0;
	u32	offset = 0;

	// PHYS ALLOCATIONS
	u32	code_pages = (code_size + PAGE_SIZE - 1) 		 / PAGE_SIZE;
	u32	code_tsize = (code_pages  + PAGES_PER_TABLE - 1) / PAGES_PER_TABLE;
	u32	data_pages = (data_size   + PAGE_SIZE - 1) 		 / PAGE_SIZE;
	u32	data_tsize = (data_pages  + PAGES_PER_TABLE - 1) / PAGES_PER_TABLE;
 	// Note: + 2 accounts for the directory and stack table
	u32	total_blocks = code_pages + data_pages + code_tsize + data_tsize + 2;


	u32	phys_page_dir	= (u32)pmm_alloc_blocks(total_blocks);
	u32	phys_code_table = phys_page_dir    + PAGE_SIZE;
	u32	phys_data_table = phys_code_table  + PAGE_SIZE * code_tsize;
	u32	phys_stack_table= phys_data_table  + PAGE_SIZE * data_tsize;
	u32	phys_code_start = phys_stack_table + PAGE_SIZE;
	u32	phys_data_start = phys_code_start  + PAGE_SIZE * code_pages;

	if (!phys_page_dir) return 0;

	spinlock_lock(&sl_kernel);

	// INIT TABLES
	pd_entry*	dir		= (pd_entry*)vmm_find_next_frees_kernel(total_blocks);
	pt_entry*	virt_code_table = (pt_entry*)((u32)dir + PAGE_SIZE);
	pt_entry*	virt_data_table = (pt_entry*)((u32)virt_code_table + PAGE_SIZE * code_tsize);
	pt_entry*	virt_stack_table= (pt_entry*)((u32)virt_data_table + PAGE_SIZE * data_tsize);
	u32		virt_code_start	= (u32)virt_stack_table + PAGE_SIZE;
	u32		virt_data_start	= virt_code_start + PAGE_SIZE * code_pages;	

	if (!dir)
	{
		spinlock_unlock(&sl_kernel);
		pmm_free_blocks(phys_page_dir, total_blocks);
		return 0;
	}

	for (i = 0; i < total_blocks * PAGE_SIZE; i+=PAGE_SIZE)
	{
		vmm_map_kpage(phys_page_dir + i, (u32)dir + i);
	}

	memset(dir, 0, PAGE_SIZE * total_blocks);

	// INIT DIR
	// Note: PROCESS_CODE_START and PROCESS_STACK_START are macros
	// Round up to next 4MB boundary
	u32	process_data_start	 = (PROCESS_CODE_START & -PTABLE_ADDR_SPACE_SIZE) + PTABLE_ADDR_SPACE_SIZE;
	u32	process_code_table_index = PAGE_DIR_INDEX(PROCESS_CODE_START);
	u32	process_data_table_index = PAGE_DIR_INDEX(process_data_start);
	u32	process_stack_table_index= PAGE_DIR_INDEX(PROCESS_STACK_START);
	u32	process_code_page_index  = PAGE_TAB_INDEX(PROCESS_CODE_START);

	dir[0] = PAGE_DIR[0];
	dir[1023] = (u32)phys_page_dir | PE_PRESENT | PE_WRITABLE;

	for (offset = 0, i = process_code_table_index;
		i < process_code_table_index + code_tsize;
		offset++, i++)
	{
		dir[i] = (phys_code_table + offset * PAGE_SIZE) | PE_PRESENT | PE_WRITABLE | PE_USER;
	}


	for (offset = 0, i = process_data_table_index;
		i < process_data_table_index + data_tsize;
		offset++, i++)
	{
		dir[i] = (phys_data_table + offset * PAGE_SIZE) | PE_PRESENT | PE_WRITABLE | PE_USER;
	}

	// Note: We don't allocate stack pages here, they will be allocated lazily.
	dir[process_stack_table_index] = phys_stack_table | PE_PRESENT | PE_WRITABLE | PE_USER;

	for (i = KPD_ENTRIES_START; i < TABLES_PER_DIR - 1; i++)
	{
		dir[i] = PAGE_DIR[i];
	}

	// INIT CODE SECTION

	for (i = 0; i < code_pages; i++)
	{
		u32	phys = phys_code_start + i * PAGE_SIZE;
		u32*	virt = (u32*)(virt_code_start + i * PAGE_SIZE);

		virt_code_table[i + process_code_page_index] = phys | PE_PRESENT | PE_USER;
		
		for (u32 j = 0; j < PAGE_SIZE / 4; j++)
		{
			virt[j] = code[i * (PAGE_SIZE / 4) + j];
		}
	}
	
	// INIT DATA SECTION
	for (i = 0; i < data_pages; i++)
	{
		u32		phys = phys_data_start + i * PAGE_SIZE;
		pt_entry*	virt = (pt_entry*)(virt_data_start + i * PAGE_SIZE);

		// Note: We don't add an offset since it should be at index 0 of that table.
		virt_data_table[i] = phys | PE_PRESENT | PE_WRITABLE | PE_USER;
		
		for (u32 j = 0; j < PAGE_SIZE / 4; j++)
		{
			virt[j] = data[i * (PAGE_SIZE / 4) + j];
		}
	}

	vmm_unmap_pages((u32)dir, total_blocks);

	spinlock_unlock(&sl_kernel);

	return phys_page_dir;
}

u32
vmm_virt_to_phys(void* virt_addr)
{
	u32	virt		= (u32)virt_addr;
	u32	pd_index	= PAGE_DIR_INDEX(virt);
	u32	pt_index	= PAGE_TAB_INDEX(virt);
	u32	entry		= PAGE_TABLES[pd_index][pt_index];

	return (entry & ~0xFFF) | (virt & 0xFFF);
}

// Note: Pages have to be read anyway so useless flag
#define PROT_READ	0
// Note: Pages that can be read (all of them) are executable by default.
// It is possible to implement specific PAGEEXEC protection but
// it would trigger page faults to do the check and is IMO costly in terms of performance.
// Check https://pax.grsecurity.net/docs/pageexec.txt for more infos.
#define PROT_EXEC	0
#define PROT_WRITE	2
#define PROT_NONE	4

// Note: if unset, map is private
#define MAP_SHARED	1
#define MAP_ANONYMOUS	2
#define MAP_DENYWRITE	4
#define MAP_EXECUTABLE	8
#define MAP_FILE	16
#define MAP_FIXED	32
#define MAP_GROWSDOWN	64
#define MAP_POPULATE	128
#define MAP_STACK	256
#define MAP_SYNC	512
#define MAP_UNINIT	1024

#define MMAP_ERROR	((u32)-1)

u32
mmap_user(
	u32 addr,
	u32 len,
	u32 prot,
	u32 flags,
	u32 fd,
	u32 off)
{
	if (!len)
	{
		// TODO: set errno
		return MMAP_ERROR;
	}

	len = len / PAGE_SIZE + (len % PAGE_SIZE ? 1 : 0);

	if (!addr)
	{
		addr = vmm_find_next_frees_user(len);
	}
	else
	{
		addr = (addr & ~0xFFF) == addr ? addr : (addr & ~0xFFF) + PAGE_SIZE;

		for (u32 i = 0; i < len; i++)
		{
			if (PAGE_TABLES[PAGE_DIR_INDEX(addr)][PAGE_TAB_INDEX(addr) + i])
			{
				// TODO: Check hint and select a new addr and handle file mapping
				addr = vmm_find_next_frees_user(len);
				break;
			}

		}
	}

	if (!addr) return MMAP_ERROR;

	// Note: Do not set user flag so that we can trigger a page fault.
	prot = prot & PROT_NONE ? PE_PRESENT : prot | PE_PRESENT;
	for (u32 i = 0; i < len; i++)
	{
		// Note: Lazy allocation. We'll add the frame when the user tries to access the page.
		PAGE_TABLES[PAGE_DIR_INDEX(addr)][PAGE_TAB_INDEX(addr) + i] = prot;
	}

	return addr;
}

u32
munmap_user(u32 addr, u32 len)
{
	// TODO: set errno
	// Note: check range
	if (addr < 0x100000 || addr + len > 0xC0000000) return MMAP_ERROR;

	len = len / PAGE_SIZE + (len % PAGE_SIZE ? 1 : 0);
	vmm_unmap_pages(addr, len);

	return 0;
}
