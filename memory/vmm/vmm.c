#include "vmm.h"
#include "../../lib/stdio/stdio.h"

p_dir*   _current_dir = 0;

extern void enable_paging();
extern void switch_dir(uint32_t dir);
extern void flush_tlb_entry(uint32_t addr);

static int
vmm_alloc_page(pt_entry* entry, uint32_t user)
{
	void* p = pmm_alloc_block();
	if (!p) return 0;
	pt_entry_set_frame(entry, (uint32_t)p);
	pt_entry_add_attrib(entry, I86_PTE_PRESENT | I86_PTE_WRITABLE | user);
	return 1;
}

static void
vmm_free_page(pt_entry* entry, uint32_t user)
{
	void* p = (void*)pt_entry_pfn(entry);
	if (p) pmm_free_block(p);
	pt_entry_del_attrib(entry, I86_PTE_PRESENT | I86_PTE_WRITABLE | user);
}

static pt_entry*
vmm_ptable_lookup_entry(p_table* p, uint32_t addr)
{
	if (p) return &p->m_entries[PAGE_TAB_INDEX(addr)];
	return 0;
}

static pd_entry*
vmm_pdir_lookup_entry(p_dir* p, uint32_t addr)
{
	if (p) return &p->m_entries[PAGE_DIR_INDEX(addr)];
	return 0;
}

void
vmm_switch_pdir(p_dir* dir)
{
	_current_dir = dir;
	switch_dir((uint32_t)_current_dir);
}

static void
vmm_map_page(p_dir* page_dir, void* phys, void* virt, uint32_t flags)
{
	pd_entry* e = &page_dir->m_entries[PAGE_DIR_INDEX((uint32_t)virt)];
	if (!(*e & I86_PTE_PRESENT))
	{
		p_table* table = (p_table*)pmm_alloc_block();
		if (!table) return;
		memset(table, 0, sizeof(p_table));
		pd_entry* entry = &page_dir->m_entries[PAGE_DIR_INDEX((uint32_t)virt)];
		pd_entry_add_attrib(entry, I86_PDE_PRESENT | I86_PDE_WRITABLE | (flags & I86_PDE_USER));
		pd_entry_set_frame(entry, (uint32_t)table);
	}
	p_table* table = (p_table*)PAGE_PHYS_ADDR(e);
	pt_entry* page = &table->m_entries[PAGE_TAB_INDEX((uint32_t)virt)];
	pt_entry_add_attrib(page, flags);
	pt_entry_set_frame(page, (uint32_t)phys);
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
    for (; i < PAGES_PER_DIR; i++)
	{
        if (pd_entry_is_present(_current_dir->m_entries[i]))
		{
            pt_entry* entry = (pt_entry*)pd_entry_pfn(_current_dir->m_entries[i]);
            for (size_t j = 0; j < PAGES_PER_TABLE; j++)
			{
                if (!pt_entry_is_present(entry[j])) return build_virt_addr(i, j);
            }
        }
		else
		{
            p_table* new_table = (p_table*)pmm_alloc_block();
            if (new_table == 0) return NULL;
            memset(new_table, 0, sizeof(p_table));
            pd_entry* new_entry = &(_current_dir->m_entries[i]);
            pd_entry_add_attrib(new_entry, I86_PDE_PRESENT | I86_PTE_WRITABLE | user);
            pd_entry_set_frame(new_entry, (uint32_t)new_table);
            return build_virt_addr(i, 0);
        }
    }
    return NULL;
}

static uint32_t
vmm_find_next_free_s(size_t nb_blocks, uint32_t user)
{
    if (nb_blocks == 1) return vmm_find_next_free(user);
    if (nb_blocks > PAGES_PER_TABLE) return 0;

	size_t i = user ? 0 : KPD_ENTRIES_START;
    for (; i < PAGES_PER_DIR; i++)
	{
        if (pd_entry_is_present(_current_dir->m_entries[i]))
		{
            pt_entry* entry = (pt_entry*)pd_entry_pfn(_current_dir->m_entries[i]);
            for (size_t j = 0; j < PAGES_PER_TABLE; j++)
			{
                if (pt_entry_is_present(entry[j])) continue;

		        for (size_t k = 1; k < PAGES_PER_TABLE - j; k++)
				{
		            if (!pt_entry_is_present(entry[j + k]) && k + 1 >= nb_blocks) return build_virt_addr(i, j);
		            else if (pt_entry_is_present(entry[j + k])) break;
		        }
            }
        }
		else
		{
            p_table* new_table = (p_table*)pmm_alloc_block();
            if (new_table == 0) return 0;
            memset(new_table, 0, sizeof(p_table));
            pd_entry* new_entry = &(_current_dir->m_entries[i]);
            pd_entry_add_attrib(new_entry, I86_PDE_PRESENT | I86_PTE_WRITABLE | user);
            pd_entry_set_frame(new_entry, (uint32_t)new_table);
            return build_virt_addr(i, 0);
        }
    }
    return NULL;
}

void
vmm_set_pdir(p_dir* dir)
{
	if (!dir) return;
	_current_dir = dir;
}

p_dir*
vmm_get_dir()
{
	return _current_dir;
}

static void*
vmm_temp_map(uint32_t phys, uint32_t nb_blocks)
{
    uint32_t virtual_addr = (uint32_t)vmm_find_next_free_s(nb_blocks, I86_PTE_KERNEL);
    if (virtual_addr == 0) return 0;
    uint32_t pd_index = PAGE_DIR_INDEX(virtual_addr);
    uint32_t pt_index = PAGE_TAB_INDEX(virtual_addr);
    pt_entry* page_table = (pt_entry*)pd_entry_pfn(_current_dir->m_entries[pd_index]);
    for (size_t i = 0; i < nb_blocks; i++)
	{
		pt_entry_set_frame(&page_table[pt_index + i], phys + i * PAGE_SIZE);
		pt_entry_add_attrib(&page_table[pt_index + i], I86_PTE_PRESENT | I86_PTE_WRITABLE);
    }
    return (void*)virtual_addr;
}

static void
vmm_temp_unmap(uint32_t virtual_addr, uint32_t nb_blocks)
{
    uint32_t pd_index = PAGE_DIR_INDEX(virtual_addr);
    uint32_t pt_index = PAGE_TAB_INDEX(virtual_addr);
    pt_entry* page_table = (pt_entry*)pd_entry_pfn(_current_dir->m_entries[pd_index]);
    for (size_t i = 0; i < nb_blocks; i++)
	{
		pt_entry_del_attrib(&page_table[pt_index + i], I86_PTE_PRESENT | I86_PTE_WRITABLE);
    }
}

void*
vmm_alloc_blocks(size_t nb_blocks, uint32_t user)
{
    uint32_t virtual_addr = (uint32_t)vmm_find_next_free_s(nb_blocks, user);
    if (virtual_addr == 0) return 0;
    uint32_t pd_index = PAGE_DIR_INDEX(virtual_addr);
    uint32_t pt_index = PAGE_TAB_INDEX(virtual_addr);
    pt_entry* page_table = (pt_entry*)pd_entry_pfn(_current_dir->m_entries[pd_index]);
    for (size_t i = 0; i < nb_blocks; i++)
	{
        vmm_alloc_page(&page_table[pt_index + i], user);
    }
    return (void*)virtual_addr;
}

void
vmm_free_blocks(uint32_t virtual_addr, uint32_t nb_blocks, uint32_t user)
{
    uint32_t pd_index = PAGE_DIR_INDEX(virtual_addr);
    uint32_t pt_index = PAGE_TAB_INDEX(virtual_addr);
    pt_entry* page_table = (pt_entry*)pd_entry_pfn(_current_dir->m_entries[pd_index]);
    for (size_t i = 0; i < nb_blocks; i++)
	{
        vmm_free_page(&page_table[pt_index + i], user);
    }
}

void
vmm_set_flags_pages(uint32_t virt_addr, uint32_t nb_blocks, uint32_t flags, uint8_t set)
{
    uint32_t pd_index = PAGE_DIR_INDEX(virt_addr);
    uint32_t pt_index = PAGE_TAB_INDEX(virt_addr);
    pt_entry* page_table = (pt_entry*)pd_entry_pfn(_current_dir->m_entries[pd_index]);
    if (set)
	{
    for (size_t i = 0; i < nb_blocks; i++)
		{
	        pt_entry_add_attrib(&page_table[i + pt_index], flags);
	    }
	}
    else
	{
	    for (size_t i = 0; i < nb_blocks; i++)
		{
	        pt_entry_del_attrib(&page_table[i + pt_index], flags);
	    }
	}
}


extern uint32_t start_kernel_virt;
extern uint32_t start_kernel;

void
vmm_init()
{
	p_table* table = (p_table*)pmm_alloc_block();
	if (!table) return;
	p_table* table2 = (p_table*)pmm_alloc_block();
	if (!table2)
	{
		pmm_free_block(table);
		return;
	}
	memset(table, 0, sizeof(p_table));
	memset(table2, 0, sizeof(p_table));
	// Note: 1st 4mb are idenitity mapped
	for (int i=0, frame=0x0, virt=0x00000000; i<PAGES_PER_TABLE; i++, frame+=PAGE_SIZE, virt+=PAGE_SIZE)
	{
 		// Note: create a new page
		pt_entry page=0;
		pt_entry* page_addr = &page;
		if (frame) pt_entry_add_attrib(page_addr, I86_PTE_PRESENT | I86_PTE_WRITABLE);
		else pt_entry_add_attrib(page_addr, I86_PTE_PRESENT);
		pt_entry_set_frame (page_addr, frame);

		// Note: ...and add it to the page table
		table2->m_entries [PAGE_TAB_INDEX(virt)] = page;
	}
		// Note: map 1mb to 3gb (where we are at)
	for (uint32_t i=0, frame=&start_kernel, virt=&start_kernel_virt; i<PAGES_PER_TABLE; i++, frame+=PAGE_SIZE, virt+=PAGE_SIZE)
	{
		// Note: create a new page
		pt_entry page=0;
		pt_entry* page_addr = &page;
		pt_entry_add_attrib(page_addr, I86_PTE_PRESENT | I86_PTE_WRITABLE);
		pt_entry_set_frame (page_addr, frame);

		// Note: ...and add it to the page table
		table->m_entries [PAGE_TAB_INDEX(virt)] = page;
	}
	// Note: create default directory table
	p_dir*	dir = (p_dir*) pmm_alloc_blocks(3);
	if (!dir)
	{
		pmm_free_block(table);
		pmm_free_block(table2);
		return;
	}
 
	// Note: clear directory table and set it as current
	memset(dir, 0, sizeof(p_dir));
	pd_entry* entry = &dir->m_entries[PAGE_DIR_INDEX ((uint32_t)&start_kernel_virt)];
	pd_entry_add_attrib(entry, I86_PDE_PRESENT | I86_PDE_WRITABLE);
	pd_entry_set_frame(entry, (uint32_t)table);

	pd_entry* entry2 = &dir->m_entries[PAGE_DIR_INDEX(0x00000000)];
	pd_entry_add_attrib(entry2, I86_PDE_PRESENT | I86_PDE_WRITABLE);
	pd_entry_set_frame(entry2, (uint32_t)table2);

	// Note: switch to our page directory
	vmm_switch_pdir(dir);
}

p_dir*
vmm_setup_process(uint32_t code_size, uint32_t data_size, uint32_t* code, uint32_t* data)
{
	// Note: INIT DIR
	uint32_t dir_raw = pmm_alloc_blocks(3);
	if (!dir_raw) return 0;
	p_dir*	dir = (p_dir*)vmm_temp_map(dir_raw, 3);
	if (!dir)
	{
		pmm_free_blocks(dir_raw, 3);
		return 0;
	}
	
	for (uint32_t i = 0; i < sizeof(p_dir); i++)
	{
		((uint8_t*)dir)[i] = 0;
	}

	dir->m_entries[0] = _current_dir->m_entries[0];


	for (uint32_t i = KPD_ENTRIES_START; i < KPD_ENTRIES_END; i++)
	{
		dir->m_entries[i] = _current_dir->m_entries[i];
	}

	// Note: CODE BLOCK
	uint32_t	code_pages = (code_size + PAGE_SIZE - 1) / PAGE_SIZE;
	uint32_t	code_block = (uint32_t)pmm_alloc_blocks(code_pages);
	if (!code_block)
	{
		vmm_free_blocks(dir, 3, I86_PTE_KERNEL);
		return 0;
	}

	uint32_t*	code_map = (uint32_t*)vmm_temp_map(code_block, code_pages);

	for (uint32_t i = 0; i < code_pages; i++)
	{
		vmm_map_page(dir, code_block + i * PAGE_SIZE, PROCESS_CODE_START + i * PAGE_SIZE, I86_PDE_PRESENT | I86_PDE_USER);
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
		vmm_free_blocks(dir, 3, I86_PTE_KERNEL);
		pmm_free_blocks(code_block, code_pages);
		return 0;
	}

	uint32_t*	data_map = (uint32_t*)vmm_temp_map(data_block, data_pages);

	for (uint32_t i = 0; i < data_pages; i++)
	{
		vmm_map_page(dir, data_block + i * PAGE_SIZE, data_virt + i * PAGE_SIZE, I86_PDE_PRESENT | I86_PDE_WRITABLE | I86_PDE_USER);
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
		vmm_free_blocks(dir, 3, I86_PTE_KERNEL);
		pmm_free_blocks(code_block, code_pages);
		pmm_free_blocks(data_block, data_pages);
		return 0;
	}

	for (uint32_t i = 0; i < data_pages; i++)
	{
		vmm_map_page(dir, stack_block + i * PAGE_SIZE, PROCESS_STACK_START + i * PAGE_SIZE, I86_PDE_PRESENT | I86_PDE_WRITABLE | I86_PDE_USER);
	}

	return dir_raw;	
}
