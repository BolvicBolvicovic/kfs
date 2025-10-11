#include "vmm.h"
#include "../../lib/stdio/stdio.h"

extern void switch_dir(uint32_t dir);
extern void flush_tlb_entry(uint32_t addr);

// Note: A pointer to the page directory, treated as an array of 1024 entries.
static pd_entry*	page_directory = (uint32_t*)RECURSIVE_PAGEDIR_ADDR;

// Note: A pointer to the page tables, treated as a 2D array.
// Accessing page_tables[pd_index][pt_index] gives you the PTE.
static pt_entry	(*page_tables)[1024] = (uint32_t(*)[1024])RECURSIVE_PAGETABLES_ADDR;

void
init_vmm(void)
{
	// Note: Alloc all kernel page tables. We sub 2 because the first and the last are already allocated.
	pd_entry	kphys_tables = pmm_alloc_blocks(TABLES_PER_DIR - KPD_ENTRIES_START - 2);

	// Note: Map them.
	for (uint32_t i = KPD_ENTRIES_START + 1; i < TABLES_PER_DIR - 1; i++, kphys_tables += PAGE_SIZE)
	{
		page_directory[i] = kphys_tables | PE_PRESENT | PE_WRITABLE;
	}

	// Note: reload page dir
	switch_dir(page_directory[1023]);
}

static void
vmm_map_kpage(uint32_t phys, uint32_t virt)
{
	uint32_t	pd_index	= PAGE_DIR_INDEX(virt);
	uint32_t	pt_index	= PAGE_TAB_INDEX(virt);
	
	page_tables[pd_index][pt_index] = phys | PE_PRESENT | PE_WRITABLE;
	flush_tlb_entry((uint32_t)virt);
}

static void
vmm_lazy_map_upage(pd_entry* dir, uint32_t virt)
{
	uint32_t	pd_index	= PAGE_DIR_INDEX(virt);

	if (!(dir[pd_index] & PE_PRESENT))
	{
		dir[pd_index] = pmm_alloc_block() | PE_PRESENT | PE_WRITABLE | PE_USER;
	}
}

static void
vmm_unmap_pages(uint32_t virt, uint32_t total_virt_blocks)
{
	uint32_t	pd_index	= PAGE_DIR_INDEX(virt);
	uint32_t	pt_index	= PAGE_TAB_INDEX(virt);

	for (uint32_t i = 0; i < total_virt_blocks; i++)
	{
		page_tables[pd_index + i / PAGES_PER_TABLE][pt_index + i % PAGES_PER_TABLE] = 0;
		flush_tlb_entry(virt + i * PAGE_SIZE);
	}
}

static void
vmm_free_page(pt_entry* entry)
{
	void* p = (void*)((uint32_t)entry & ~0xFFF);
	if (p) pmm_free_block(p);
	*entry = 0;
}

static uint32_t
build_virt_addr(uint32_t pd_index, uint32_t pt_index)
{
    return (pd_index << 22) | (pt_index << 12);
}

static uint32_t
vmm_find_next_free(uint32_t user)
{
	size_t i = user ? 0 : KPD_ENTRIES_START;

    for (; i < TABLES_PER_DIR; i++)
	{
        if (page_directory[i] & PE_PRESENT)
		{
            pt_entry* entry = &page_tables[i][0];
            for (size_t j = 0; j < PAGES_PER_TABLE; j++)
			{
                if (!(entry[j] & PE_PRESENT)) return build_virt_addr(i, j);
            }
        }
		else
		{
            return build_virt_addr(i, 0);
        }
    }
    return 0;
}

static uint32_t
vmm_find_next_free_s(size_t nb_blocks, uint32_t user)
{
    if (nb_blocks == 1) return vmm_find_next_free(user);
    if (nb_blocks > PAGES_PER_TABLE) return 0;

	size_t i = user ? 0 : KPD_ENTRIES_START;
    for (; i < TABLES_PER_DIR; i++)
	{
        if (page_directory[i] & PE_PRESENT)
		{
            pt_entry* entry = &page_tables[i][0];
            for (size_t j = 0; j < PAGES_PER_TABLE; j++)
			{
                if (entry[j] & PE_PRESENT) continue;

		        for (size_t k = 1; k < PAGES_PER_TABLE - j; k++)
				{
		            if (!(entry[j + k] & PE_PRESENT) && k + 1 >= nb_blocks) return build_virt_addr(i, j);
		            else if (entry[j + k] & PE_PRESENT) break;
		        }
            }
        }
		else
		{
            return build_virt_addr(i, 0);
        }
    }
    return 0;
}

void*
vmm_alloc_blocks(size_t nb_blocks, uint32_t user)
{
    uint32_t virtual_addr	= (uint32_t)vmm_find_next_free_s(nb_blocks, user);
    if (virtual_addr == 0) return 0;

	if (user)
	{
    	for (size_t i = 0; i < nb_blocks; i+=PAGES_PER_TABLE)
		{
			vmm_lazy_map_upage(page_directory, virtual_addr + i);
    	}
	}
	else
	{
    	for (size_t i = 0; i < nb_blocks; i++)
		{
			uint32_t physical_addr = pmm_alloc_block();
			if (!physical_addr)
			{
				vmm_free_blocks(virtual_addr, i);
				return 0;
			}
			vmm_map_kpage(physical_addr, virtual_addr + (i * PAGE_SIZE));
    	}
	}

    return (void*)virtual_addr;
}

void
vmm_free_blocks(uint32_t virtual_addr, uint32_t nb_blocks)
{
    uint32_t	pd_index	= PAGE_DIR_INDEX(virtual_addr);
    uint32_t	pt_index	= PAGE_TAB_INDEX(virtual_addr);
    pt_entry*	table		= &page_tables[pd_index][pt_index];

    for (size_t i = 0; i < nb_blocks; i++)
	{
        vmm_free_page(&table[i]);
    }
	flush_tlb_entry((uint32_t)virtual_addr);
}

void
vmm_set_flags_pages(uint32_t virt_addr, uint32_t nb_blocks, uint32_t flags, uint8_t set)
{
    uint32_t	pd_index	= PAGE_DIR_INDEX(virt_addr);
    uint32_t	pt_index	= PAGE_TAB_INDEX(virt_addr);
    pt_entry*	table		= &page_tables[pd_index][pt_index];
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

pd_entry*
vmm_setup_process(uint32_t code_size, uint32_t data_size, uint32_t* code, uint32_t* data)
{
	uint32_t	i;
	uint32_t	offset;

	// PHYS ALLOCATIONS
	uint32_t	code_pages = code_size   / PAGE_SIZE 	   + 1;										// OK
	uint32_t	code_tsize = code_pages  / PAGES_PER_TABLE + 1;										// OK
	uint32_t	data_pages = data_size   / PAGE_SIZE 	   + 1;										// OK
	uint32_t	data_tsize = data_pages  / PAGES_PER_TABLE + 1;										// OK
	uint32_t	stack_tsize= STACK_PAGES / PAGES_PER_TABLE + 1;										// OK
	uint32_t	table_nbs  = code_tsize + data_tsize + stack_tsize + 1; 							// OK Note: + 1 accounts for the directory
	uint32_t	page_nbs   = code_pages + data_pages + STACK_PAGES;									// OK

	uint32_t	total_phys_blocks = page_nbs + table_nbs;											// OK
	uint32_t	total_virt_blocks = total_phys_blocks - STACK_PAGES;								// OK

	uint32_t	phys_page_dir	= (uint32_t)pmm_alloc_blocks(total_phys_blocks);					// OK
	uint32_t	phys_code_table = phys_page_dir    + PAGE_SIZE;										// OK
	uint32_t	phys_data_table = phys_code_table  + PAGE_SIZE * code_tsize;						// OK
	uint32_t	phys_stack_table= phys_data_table  + PAGE_SIZE * data_tsize;						// OK
	uint32_t	phys_code_start = phys_stack_table + PAGE_SIZE * stack_tsize;						// OK
	uint32_t	phys_data_start = phys_code_start  + PAGE_SIZE * code_pages;						// OK
	uint32_t	phys_stack_start= phys_data_start  + PAGE_SIZE * data_pages;						// OK

	if (!phys_page_dir) return 0;																	// OK

	// INIT TABLES
	pd_entry*	dir	= (pd_entry*)vmm_find_next_free_s(total_virt_blocks, PE_KERNEL);				// OK
	pt_entry*	virt_code_table = (pt_entry*)((uint32_t)dir + PAGE_SIZE);							// OK
	pt_entry*	virt_data_table = (pt_entry*)((uint32_t)virt_code_table + PAGE_SIZE * code_tsize);	// OK
	pt_entry*	virt_stack_table= (pt_entry*)((uint32_t)virt_data_table + PAGE_SIZE * data_tsize);	// OK
	uint32_t	virt_code_start	= (uint32_t)virt_stack_table + PAGE_SIZE * stack_tsize;				// OK
	uint32_t	virt_data_start	= virt_code_start + PAGE_SIZE * code_pages;							// OK

	if (!dir)																						// OK
	{
		pmm_free_blocks((void*)phys_page_dir, total_phys_blocks);									// OK
		return 0;																					// OK
	}

	for (i = 0; i < total_virt_blocks * PAGE_SIZE; i+=PAGE_SIZE)
	{
		vmm_map_kpage(phys_page_dir + i, (uint32_t)dir + i);										// OK
	}

	memset(dir, 0, PAGE_SIZE * total_virt_blocks);													// OK
	
	// INIT DIR
	// Note: PROCESS_CODE_START and PROCESS_STACK_START are macros
	// Round up to next 4MB boundary
	uint32_t	process_data_start		 = ((PROCESS_CODE_START + code_size + PTABLE_ADDR_SPACE_SIZE - 1) & ~(PTABLE_ADDR_SPACE_SIZE - 1));
	uint32_t	process_code_table_index = PAGE_DIR_INDEX(PROCESS_CODE_START);						// OK 
	uint32_t	process_data_table_index = PAGE_DIR_INDEX(process_data_start);						// OK
	uint32_t	process_stack_table_index= PAGE_DIR_INDEX(PROCESS_STACK_START);						// OK
	uint32_t	process_code_page_index  = PAGE_TAB_INDEX(PROCESS_CODE_START);						// OK
	uint32_t	process_stack_page_index = PAGE_TAB_INDEX(PROCESS_STACK_START);						// OK

	dir[0] = page_directory[0];																		// OK
	dir[1023] = (uint32_t)phys_page_dir | PE_PRESENT | PE_WRITABLE;									// OK

	for (offset = 0, i = process_code_table_index;													// OK
		i < process_code_table_index + code_tsize;													// OK
		offset++, i++)																				// OK
	{
		dir[i] = (phys_code_table + offset * PAGE_SIZE) | PE_PRESENT | PE_WRITABLE | PE_USER;		// OK
	}


	for (offset = 0, i = process_data_table_index;													// OK
		i < process_data_table_index + data_tsize;													// OK
		offset++, i++)																				// OK
	{
		dir[i] = (phys_data_table + offset * PAGE_SIZE) | PE_PRESENT | PE_WRITABLE | PE_USER;		// OK
	}

	for (offset = 0, i = process_stack_table_index;													// OK
		i < process_stack_table_index + stack_tsize;												// OK
		offset++, i++)																				// OK
	{
		dir[i] = (phys_stack_table + offset * PAGE_SIZE) | PE_PRESENT | PE_WRITABLE | PE_USER;		// OK
	}

	for (i = KPD_ENTRIES_START; i < TABLES_PER_DIR - 1; i++)										// OK
	{
		dir[i] = page_directory[i];																	// OK
	}

	// INIT CODE SECTION

	for (i = 0; i < code_pages; i++)
	{
		uint32_t	phys = phys_code_start + i * PAGE_SIZE;											// OK
		uint32_t*	virt = (uint32_t*)(virt_code_start + i * PAGE_SIZE);							// OK

		virt_code_table[i + process_code_page_index] = phys | PE_PRESENT | PE_WRITABLE | PE_USER;
		
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
	
	// INIT (STACK) BSS SECTION
	for (i = 0; i < STACK_PAGES; i++)
	{
		uint32_t	phys = phys_stack_start + i * PAGE_SIZE;
		virt_stack_table[i + process_stack_page_index] = phys | PE_PRESENT | PE_WRITABLE | PE_USER;
	}

	vmm_unmap_pages((uint32_t)dir, total_virt_blocks);
	return (pd_entry*)phys_page_dir;
}
