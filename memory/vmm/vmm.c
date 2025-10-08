#include "vmm.h"
#include "../../lib/stdio/stdio.h"

extern void switch_dir(uint32_t dir);
extern void flush_tlb_entry(uint32_t addr);

// A pointer to the page directory, treated as an array of 1024 entries.
volatile pd_entry*	page_directory = (uint32_t*)RECURSIVE_PAGEDIR_ADDR;

// A pointer to the page tables, treated as a 2D array.
// Accessing page_tables[pd_index][pt_index] gives you the PTE.
volatile pt_entry	(*page_tables)[1024] = (uint32_t(*)[1024])RECURSIVE_PAGETABLES_ADDR;

pd_entry*			_current_dir = 0;
void*
vmm_temp_unmap(uint32_t virtual_addr, uint32_t nb_blocks);
void*
vmm_temp_map(uint32_t physical_addr, uint32_t nb_blocks);

void
vmm_switch_pdir(pd_entry* dir)
{
	_current_dir = dir;
	switch_dir((uint32_t)_current_dir);
}

static void
vmm_map_page(pd_entry* dir, void* phys, void* virt, uint32_t flags)
{
	uint32_t	pd_index	= PAGE_DIR_INDEX((uint32_t)virt);
	uint32_t	pt_index	= PAGE_TAB_INDEX((uint32_t)virt);
	pd_entry*	entry		= &dir[pd_index];
	pt_entry*	table_virt	= 0;
	uint8_t	mapped_temp	= 0;
	uint32_t	table_phys;
	uint32_t	table_flags	= PE_PRESENT | PE_WRITABLE;

	if (flags & PE_USER) table_flags |= PE_USER;

	if (!(*entry & PE_PRESENT))
	{
		table_phys = (uint32_t)pmm_alloc_block();
		if (!table_phys) return;
		*entry = table_phys | table_flags;

		if (dir == page_directory)
		{
			table_virt = (pt_entry*)&page_tables[pd_index][0];
		}
		else
		{
			table_virt = (pt_entry*)vmm_temp_map(table_phys, 1);
			if (!table_virt)
			{
				*entry = 0;
				pmm_free_block((void*)table_phys);
				return;
			}
			mapped_temp = 1;
		}

		memset(table_virt, 0, PAGES_PER_TABLE * sizeof(pt_entry));
	}
	else
	{
		table_phys = *entry & ~0xFFF;
		if (dir == page_directory)
		{
			table_virt = (pt_entry*)&page_tables[pd_index][0];
		}
		else
		{
			table_virt = (pt_entry*)vmm_temp_map(table_phys, 1);
			if (!table_virt) return;
			mapped_temp = 1;
		}
	}

	table_virt[pt_index] = (uint32_t)phys | flags;

	if (mapped_temp) vmm_temp_unmap((uint32_t)table_virt, 1);

	if (dir == page_directory) flush_tlb_entry((uint32_t)virt);
}

static void
vmm_free_page(pt_entry* entry, uint32_t user)
{
	void* p = (void*)((uint32_t)entry & ~0xFFF);
	if (p) pmm_free_block(p);
	*entry &= ~(PE_PRESENT | PE_WRITABLE | user);
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
vmm_temp_map(uint32_t physical_addr, uint32_t nb_blocks)
{
    uint32_t virtual_addr	= (uint32_t)vmm_find_next_free_s(nb_blocks, PE_KERNEL);
    if (virtual_addr == 0) return 0;

    uint32_t	pd_index	= PAGE_DIR_INDEX(virtual_addr);
    uint32_t	pt_index	= PAGE_TAB_INDEX(virtual_addr);
    pt_entry*	table		= &page_tables[pd_index][pt_index];

    for (size_t i = 0; i < nb_blocks; i++)
	{
		vmm_map_page(page_directory, physical_addr + (i * PAGE_SIZE), virtual_addr + (i * PAGE_SIZE), PE_KERNEL | PE_WRITABLE | PE_PRESENT);
    }

    return (void*)virtual_addr;
}

void*
vmm_temp_unmap(uint32_t virtual_addr, uint32_t nb_blocks)
{
    uint32_t	pd_index	= PAGE_DIR_INDEX(virtual_addr);
    uint32_t	pt_index	= PAGE_TAB_INDEX(virtual_addr);
    pt_entry*	table		= &page_tables[pd_index][pt_index];

    for (size_t i = 0; i < nb_blocks; i++)
	{
		table[i] ^= (PE_PRESENT | PE_WRITABLE);
    }
}

void*
vmm_alloc_blocks(size_t nb_blocks, uint32_t user)
{
    uint32_t virtual_addr	= (uint32_t)vmm_find_next_free_s(nb_blocks, user);
    if (virtual_addr == 0) return 0;

    uint32_t	pd_index	= PAGE_DIR_INDEX(virtual_addr);
    uint32_t	pt_index	= PAGE_TAB_INDEX(virtual_addr);
    pt_entry*	table		= &page_tables[pd_index][pt_index];

    for (size_t i = 0; i < nb_blocks; i++)
	{
		uint32_t physical_addr = pmm_alloc_block();
		if (!physical_addr)
		{
			vmm_free_blocks(virtual_addr, i, user);
			return 0;
		}
		vmm_map_page(page_directory, physical_addr, virtual_addr + (i * PAGE_SIZE), user | PE_WRITABLE | PE_PRESENT);
    }

    return (void*)virtual_addr;
}

void
vmm_free_blocks(uint32_t virtual_addr, uint32_t nb_blocks, uint32_t user)
{
    uint32_t	pd_index	= PAGE_DIR_INDEX(virtual_addr);
    uint32_t	pt_index	= PAGE_TAB_INDEX(virtual_addr);
    pt_entry*	table		= &page_tables[pd_index][pt_index];

    for (size_t i = 0; i < nb_blocks; i++)
	{
        vmm_free_page(&table[i], user);
    }
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

void
vmm_init()
{
	asm volatile ("mov %%cr3, %0": "=r"(_current_dir));
}

pd_entry*
vmm_setup_process(uint32_t code_size, uint32_t data_size, uint32_t* code, uint32_t* data)
{
	// Note: INIT DIR
	uint32_t	dir_raw	= pmm_alloc_block();
	if (!dir_raw) return 0;
	pd_entry*	dir		= (pd_entry*)vmm_temp_map(dir_raw, 1);
	if (!dir)
	{
		pmm_free_block(dir_raw);
		return 0;
	}
	
	memset(dir, 0, PAGE_SIZE);

	dir[0]		= page_directory[0];
	dir[1023]	= dir_raw | PE_PRESENT | PE_WRITABLE;

	for (uint32_t i = KPD_ENTRIES_START; i < KPD_ENTRIES_END - 1; i++)
	{
		dir[i]	= page_directory[i];
	}

	// Note: CODE BLOCK
	uint32_t	code_pages = (code_size + PAGE_SIZE - 1) / PAGE_SIZE;
	uint32_t	code_block = (uint32_t)pmm_alloc_blocks(code_pages);
	if (!code_block)
	{
		vmm_free_blocks(dir, 1, PE_KERNEL);
		return 0;
	}

	uint32_t*	code_map = (uint32_t*)vmm_temp_map(code_block, code_pages);

	for (uint32_t i = 0; i < code_pages; i++)
	{
		vmm_map_page(dir, code_block + i * PAGE_SIZE, PROCESS_CODE_START + i * PAGE_SIZE, PE_PRESENT | PE_USER);
		for (uint32_t j = 0; j < PAGE_SIZE / 4; j++)
		{
			code_map[j + (i * PAGE_SIZE) / 4] = code[j + (i * PAGE_SIZE) / 4];
		}
	}

	vmm_temp_unmap(code_block, code_pages);

	// Note: DATA BLOCK
	uint32_t	data_pages = (data_size + PAGE_SIZE - 1) / PAGE_SIZE;
	uint32_t	data_block = (uint32_t)pmm_alloc_blocks(data_pages);
	uint32_t 	data_virt  = PROCESS_CODE_START + ((code_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));
	if (!data_block)
	{
		vmm_free_blocks(dir, 1, PE_KERNEL);
		pmm_free_blocks(code_block, code_pages);
		return 0;
	}

	uint32_t*	data_map = (uint32_t*)vmm_temp_map(data_block, data_pages);

	for (uint32_t i = 0; i < data_pages; i++)
	{
		vmm_map_page(dir, data_block + i * PAGE_SIZE, data_virt + i * PAGE_SIZE, PE_PRESENT | PE_WRITABLE | PE_USER);
		for (uint32_t j = 0; j < PAGE_SIZE / 4; j++)
		{
			data_map[j + (i * PAGE_SIZE) / 4] = data[j + (i * PAGE_SIZE) / 4];
		}
	}

	vmm_temp_unmap(data_block, data_pages);

	// Note: STACK
	uint32_t	stack_block = (uint32_t)pmm_alloc_blocks(STACK_PAGES);
	if (!stack_block)
	{
		vmm_free_blocks(dir, 1, PE_KERNEL);
		pmm_free_blocks(code_block, code_pages);
		pmm_free_blocks(data_block, data_pages);
		return 0;
	}

	for (uint32_t i = 0; i < STACK_PAGES; i++)
	{
		vmm_map_page(dir, stack_block + i * PAGE_SIZE, PROCESS_STACK_START + i * PAGE_SIZE, PE_PRESENT | PE_WRITABLE | PE_USER);
	}

	return dir_raw;	
}
