#include "vmm.h"
#include "../../lib/stdio/stdio.h"

extern void switch_dir(uint32_t dir);
extern void flush_tlb_entry(uint32_t addr);

#define PAGE_DIR 	((uint32_t*)RECURSIVE_PAGEDIR_ADDR)
#define PAGE_TABLES ((uint32_t(*)[1024])RECURSIVE_PAGETABLES_ADDR)

void
init_vmm(void)
{
	// Note: Alloc all kernel page tables. We sub 2 because the first and the last are already allocated.
	pd_entry	kphys_tables = pmm_alloc_blocks(TABLES_PER_DIR - KPD_ENTRIES_START - 2);

	// Note: Map them.
	for (uint32_t i = KPD_ENTRIES_START + 1; i < TABLES_PER_DIR - 1; i++, kphys_tables += PAGE_SIZE)
	{
		PAGE_DIR[i] = kphys_tables | PE_PRESENT | PE_WRITABLE;
	}

	// Note: reload page dir
	switch_dir(PAGE_DIR[1023] & ~0xFFF);
}

static inline void
vmm_map_kpage(uint32_t phys, uint32_t virt)
{
	uint32_t	pd_index	= PAGE_DIR_INDEX(virt);
	uint32_t	pt_index	= PAGE_TAB_INDEX(virt);

	PAGE_TABLES[pd_index][pt_index] = phys | PE_PRESENT | PE_WRITABLE;
	flush_tlb_entry((uint32_t)virt);
}

static inline void
vmm_unmap_pages(uint32_t virt, uint32_t total_virt_blocks)
{
	uint32_t	pd_index	= PAGE_DIR_INDEX(virt);
	uint32_t	pt_index	= PAGE_TAB_INDEX(virt);

	for (uint32_t i = 0; i < total_virt_blocks; i++)
	{
		PAGE_TABLES[pd_index + i / PAGES_PER_TABLE][pt_index + i % PAGES_PER_TABLE] = 0;
		flush_tlb_entry(virt + i * PAGE_SIZE);
	}
}

static inline uint32_t
build_virt_addr(uint32_t pd_index, uint32_t pt_index)
{
    return (pd_index << 22) | (pt_index << 12);
}

static uint32_t
vmm_find_next_free_user(void)
{
	static uint32_t	user_dir_index = 0;
	static uint32_t user_tab_index = 0;

	uint32_t	i = user_dir_index;
	uint32_t	j = user_tab_index;

    for (; i < KPD_ENTRIES_START; i++)
	{
        if (PAGE_DIR[i] & PE_PRESENT)
		{
            for (; j < PAGES_PER_TABLE; j++)
			{
                if (!(PAGE_TABLES[i][j] & PE_PRESENT))
				{
					uint32_t	addr = build_virt_addr(i, j);
					user_dir_index = i; 
					user_tab_index = j + 1;
					return addr;
				}
            }
			j = 0;
        }
		else
		{
			uint32_t	addr = build_virt_addr(i, 0);
			PAGE_DIR[i] = pmm_alloc_block() | PE_PRESENT | PE_WRITABLE | PE_USER;
			if (PAGE_DIR[i] == (PE_PRESENT | PE_WRITABLE | PE_USER))
			{
				PAGE_DIR[i] = 0;
				return 0;
			}
			user_dir_index = i;
			user_tab_index = j + 1;
            return addr;
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

static uint32_t
vmm_find_next_free_kernel(void)
{
	static uint32_t	kernel_dir_index = KPD_ENTRIES_START;
	static uint32_t	kernel_tab_index = 0;

	uint32_t	i = kernel_dir_index;
	uint32_t	j = kernel_tab_index;

    for (; i < TABLES_PER_DIR; i++)
	{
    	for (; j < PAGES_PER_TABLE; j++)
		{
    	    if (!(PAGE_TABLES[i][j] & PE_PRESENT))
			{
				uint32_t	addr = build_virt_addr(i, j);
				kernel_dir_index = i;
				kernel_tab_index = j + 1;
				return addr;
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

static uint32_t
vmm_find_next_frees_user(size_t nb_blocks)
{
	static uint32_t	user_dir_index = 0;
	static uint32_t	user_tab_index = 0;

    if (nb_blocks == 1) return vmm_find_next_free_user();
    if (nb_blocks > PAGES_PER_TABLE) return 0;

	uint32_t	i = user_dir_index;
	uint32_t	j = user_tab_index;

    for (; i < KPD_ENTRIES_START; i++)
	{
        if (PAGE_DIR[i] & PE_PRESENT)
		{
            for (;j < PAGES_PER_TABLE; j++)
			{
                if (PAGE_TABLES[i][j] & PE_PRESENT) continue;

		        for (uint32_t k = 1; k < PAGES_PER_TABLE - j; k++)
				{
		            if (!(PAGE_TABLES[i][j + k] & PE_PRESENT) && k + 1 == nb_blocks)
					{
						uint32_t	addr = build_virt_addr(i, j + k);
						user_dir_index = i;
						user_tab_index = j + k + 1;
						return addr;
					}
		            else if (PAGE_TABLES[i][j + k] & PE_PRESENT) break;
		        }
            }

			j = 0;
        }
		else
		{
			uint32_t	addr = build_virt_addr(i, 0);
			PAGE_DIR[i] = pmm_alloc_block() | PE_PRESENT | PE_WRITABLE | PE_USER;
			if (PAGE_DIR[i] == (PE_PRESENT | PE_WRITABLE | PE_USER))
			{
				PAGE_DIR[i] = 0;
				return 0;
			}
			user_dir_index = i;
			user_tab_index = j + nb_blocks + 1;
			return addr;
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

static uint32_t
vmm_find_next_frees_kernel(size_t nb_blocks)
{
	static uint32_t	kernel_dir_index = KPD_ENTRIES_START;
	static uint32_t	kernel_tab_index = 0;

    if (nb_blocks == 1) return vmm_find_next_free_kernel();
    if (nb_blocks > PAGES_PER_TABLE) return 0;

	uint32_t	i = kernel_dir_index;
	uint32_t	j = kernel_tab_index;

    for (; i < TABLES_PER_DIR; i++)
	{
        for (;j < PAGES_PER_TABLE; j++)
		{
            if (PAGE_TABLES[i][j] & PE_PRESENT) continue;

		    for (uint32_t k = 1; k < PAGES_PER_TABLE - j; k++)
			{
		        if (!(PAGE_TABLES[i][j + k] & PE_PRESENT) && k + 1 == nb_blocks)
				{
					uint32_t	addr = build_virt_addr(i, j + k);
					kernel_dir_index = i;
					kernel_tab_index = j + k + 1;
					return addr;
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
vmm_alloc_kblocks(size_t nb_blocks)
{
    uint32_t	virtual_addr = (uint32_t)vmm_find_next_frees_kernel(nb_blocks);
    if (virtual_addr == 0) return 0;

	uint32_t	physical_addr = pmm_alloc_blocks(nb_blocks);
	if (!physical_addr) return 0;

	for (uint32_t i = 0; i < nb_blocks; i++)
	{
		vmm_map_kpage(physical_addr + (i * PAGE_SIZE), virtual_addr + (i * PAGE_SIZE));
    }

    return (void*)virtual_addr;
}

void
vmm_free_blocks(uint32_t virtual_addr, uint32_t nb_blocks)
{
    uint32_t	pd_index	= PAGE_DIR_INDEX(virtual_addr);
    uint32_t	pt_index	= PAGE_TAB_INDEX(virtual_addr);

	pmm_free_blocks(PAGE_TABLES[pd_index][pt_index] & ~0xFFF, nb_blocks);
	vmm_unmap_pages(virtual_addr, nb_blocks);
}

void
vmm_set_flags_pages(uint32_t virt_addr, uint32_t nb_blocks, uint32_t flags, uint8_t set)
{
    uint32_t	pd_index	= PAGE_DIR_INDEX(virt_addr);
    uint32_t	pt_index	= PAGE_TAB_INDEX(virt_addr);
    pt_entry*	table		= &PAGE_TABLES[pd_index][pt_index];
    if (set)
	{
    	for (size_t i = 0; i < nb_blocks; i++)
		{
	        table[i] |= flags;
	    }
	}
    else
	{
	    for (size_t i = 0; i < nb_blocks; i++)
		{
	        table[i] &= ~flags;
	    }
	}
}

uint32_t
vmm_setup_process(uint32_t code_size, uint32_t data_size, uint32_t* code, uint32_t* data)
{
	uint32_t	i = 0;
	uint32_t	offset = 0;

	// PHYS ALLOCATIONS
	uint32_t	code_pages = (code_size + PAGE_SIZE - 1) 		 / PAGE_SIZE;
	uint32_t	code_tsize = (code_pages  + PAGES_PER_TABLE - 1) / PAGES_PER_TABLE;
	uint32_t	data_pages = (data_size   + PAGE_SIZE - 1) 		 / PAGE_SIZE;
	uint32_t	data_tsize = (data_pages  + PAGES_PER_TABLE - 1) / PAGES_PER_TABLE;
 	// Note: + 2 accounts for the directory and stack table
	uint32_t	total_blocks = code_pages + data_pages + code_tsize + data_tsize + 2;


	uint32_t	phys_page_dir	= (uint32_t)pmm_alloc_blocks(total_blocks);
	uint32_t	phys_code_table = phys_page_dir    + PAGE_SIZE;
	uint32_t	phys_data_table = phys_code_table  + PAGE_SIZE * code_tsize;
	uint32_t	phys_stack_table= phys_data_table  + PAGE_SIZE * data_tsize;
	uint32_t	phys_code_start = phys_stack_table + PAGE_SIZE;
	uint32_t	phys_data_start = phys_code_start  + PAGE_SIZE * code_pages;

	if (!phys_page_dir) return 0;

	// INIT TABLES
	pd_entry*	dir	= (pd_entry*)vmm_find_next_frees_kernel(total_blocks);
	pt_entry*	virt_code_table = (pt_entry*)((uint32_t)dir + PAGE_SIZE);
	pt_entry*	virt_data_table = (pt_entry*)((uint32_t)virt_code_table + PAGE_SIZE * code_tsize);
	pt_entry*	virt_stack_table= (pt_entry*)((uint32_t)virt_data_table + PAGE_SIZE * data_tsize);
	uint32_t	virt_code_start	= (uint32_t)virt_stack_table + PAGE_SIZE;
	uint32_t	virt_data_start	= virt_code_start + PAGE_SIZE * code_pages;	

	if (!dir)
	{
		pmm_free_blocks(phys_page_dir, total_blocks);
		return 0;
	}

	for (i = 0; i < total_blocks * PAGE_SIZE; i+=PAGE_SIZE)
	{
		vmm_map_kpage(phys_page_dir + i, (uint32_t)dir + i);
	}

	memset(dir, 0, PAGE_SIZE * total_blocks);

	// INIT DIR
	// Note: PROCESS_CODE_START and PROCESS_STACK_START are macros
	// Round up to next 4MB boundary
	uint32_t	process_data_start		 = (PROCESS_CODE_START & -PTABLE_ADDR_SPACE_SIZE) + PTABLE_ADDR_SPACE_SIZE;
	uint32_t	process_code_table_index = PAGE_DIR_INDEX(PROCESS_CODE_START);
	uint32_t	process_data_table_index = PAGE_DIR_INDEX(process_data_start);
	uint32_t	process_stack_table_index= PAGE_DIR_INDEX(PROCESS_STACK_START);
	uint32_t	process_code_page_index  = PAGE_TAB_INDEX(PROCESS_CODE_START);

	dir[0] = PAGE_DIR[0];
	dir[1023] = (uint32_t)phys_page_dir | PE_PRESENT | PE_WRITABLE;

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
		uint32_t	phys = phys_code_start + i * PAGE_SIZE;
		uint32_t*	virt = (uint32_t*)(virt_code_start + i * PAGE_SIZE);

		virt_code_table[i + process_code_page_index] = phys | PE_PRESENT | PE_USER;
		
		for (uint32_t j = 0; j < PAGE_SIZE / 4; j++)
		{
			virt[j] = code[i * (PAGE_SIZE / 4) + j];
		}
	}
	
	// INIT DATA SECTION
	for (i = 0; i < data_pages; i++)
	{
		uint32_t	phys = phys_data_start + i * PAGE_SIZE;
		pt_entry*	virt = (pt_entry*)(virt_data_start + i * PAGE_SIZE);

		// Note: We don't add an offset since it should be at index 0 of that table.
		virt_data_table[i] = phys | PE_PRESENT | PE_WRITABLE | PE_USER;
		
		for (uint32_t j = 0; j < PAGE_SIZE / 4; j++)
		{
			virt[j] = data[i * (PAGE_SIZE / 4) + j];
		}
	}

	vmm_unmap_pages((uint32_t)dir, total_blocks);
	return phys_page_dir;
}
