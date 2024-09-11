#include "vmm.h"
#include "../../lib/stdio/stdio.h"

p_dir*   _current_dir = NULL;
uint32_t _cur_pdbr = 0;
extern void enable_paging();
extern void switch_dir(uint32_t dir);
extern void flush_tlb_entry(uint32_t addr);

int vmm_alloc_page(pt_entry* entry) {
	void* p = pmm_alloc_block();
	if (!p) return 0;
	pt_entry_set_frame(entry, (uint32_t)p);
	pt_entry_add_attrib(entry, I86_PTE_PRESENT | I86_PTE_WRITABLE);
	return 1;
}

void vmm_free_page(pt_entry* entry) {
	void* p = (void*)pt_entry_pfn(entry);
	if (p) pmm_free_block(p);
	pt_entry_del_attrib(entry, I86_PTE_PRESENT);
}

pt_entry* vmm_ptable_lookup_entry(p_table* p, uint32_t addr) {
	if (p) return &p->m_entries[PAGE_TAB_INDEX(addr)];
	return 0;
}

pd_entry* vmm_pdir_lookup_entry(p_dir* p, uint32_t addr) {
	if (p) return &p->m_entries[PAGE_TAB_INDEX(addr)];
	return 0;
}

int vmm_switch_pdir(p_dir* dir) {
	if (!dir) return 0;
	_current_dir = dir;
	switch_dir(_cur_pdbr);
}

p_dir* vmm_get_dir() { return _current_dir; }

void vmm_map_page(void* phys, void* virt) {
	p_dir*    page_dir = vmm_get_dir();
	pd_entry* e = &page_dir->m_entries[PAGE_DIR_INDEX((uint32_t)virt)];
	if ((*e & I86_PTE_PRESENT) != I86_PTE_PRESENT) {
		p_table* table = (p_table*)pmm_alloc_block();
		if (!table) return;
		memset(table, 0, sizeof(p_table));
		pd_entry* entry = &page_dir->m_entries[PAGE_DIR_INDEX((uint32_t)virt)];
		pd_entry_add_attrib(entry, I86_PDE_PRESENT);
		pd_entry_add_attrib(entry, I86_PDE_WRITABLE);
		pd_entry_set_frame(entry, (uint32_t)table);
	}
	p_table* table = (p_table*)PAGE_PHYS_ADDR(e);
	pt_entry* page = &table->m_entries[PAGE_TAB_INDEX((uint32_t)virt)];
	pt_entry_set_frame(page, (uint32_t)phys);
	pt_entry_add_attrib(page, I86_PTE_PRESENT);
}

uint32_t build_virt_addr(uint32_t pd_index, uint32_t pt_index) {
    return (pd_index << 22) | (pt_index << 12);
}

uint32_t* vmm_find_next_free() {
    for (size_t i = 0; i < 1024; i++) {
        if (pd_entry_is_present(_current_dir->m_entries[i])) {
            pt_entry* entry = (pt_entry*)pd_entry_pfn(_current_dir->m_entries[i]);
            for (size_t j = 0; j < 1024; j++) {
                if (!pt_entry_is_present(entry[j])) {
                    return build_virt_addr(i, j);
                }
            }
        } else {
            p_table* new_table = (p_table*)pmm_alloc_block();
            if (new_table == 0) return NULL;
            memset(new_table, 0, sizeof(p_table));
            for (size_t i = 0; i < 1024; i++) {
                pt_entry new_page = 0;
                pt_entry* new_page_p = &new_page;
                if (!vmm_alloc_page(new_page_p)) continue;
                if (i > 0) pt_entry_del_attrib(new_page_p, I86_PDE_PRESENT);
                new_table->m_entries[i] = new_page;
            }
            pd_entry* new_entry = &(_current_dir->m_entries[i]);
            pd_entry_add_attrib(new_entry, I86_PDE_PRESENT | I86_PTE_WRITABLE);
            pd_entry_set_frame(new_entry, (uint32_t)new_table);
            return build_virt_addr(i, 0);
        }
    }
    return NULL;
}

uint32_t* vmm_find_next_free_s(size_t nb_blocks) {
    if (nb_blocks == 1) return vmm_find_next_free();
    for (size_t i = 0; i < 1024; i++) {
        if (pd_entry_is_present(_current_dir->m_entries[i])) {
            pt_entry* entry = (pt_entry*)pd_entry_pfn(_current_dir->m_entries[i]);
            for (size_t j = 0; j < 1024; j++) {
                if (!pt_entry_is_present(entry[j])) {
		            for (size_t k = 1; k < 1024 - j; k++) {
			            if (!pt_entry_is_present(entry[j + k]) && k + 1 >= nb_blocks)
			                return build_virt_addr(i, j);
			            else if (pt_entry_is_present(entry[j + k])) {
                            break;
                        }
		            }
                }
            }
        } else {
            p_table* new_table = (p_table*)pmm_alloc_block();
            if (new_table == 0) return NULL;
            memset(new_table, 0, sizeof(p_table));
            for (size_t i = 0; i < 1024; i++) {
                pt_entry new_page = 0;
                pt_entry* new_page_p = &new_page;
                if (!vmm_alloc_page(new_page_p)) continue;
                if (i > nb_blocks - 1) pt_entry_del_attrib(new_page_p, I86_PDE_PRESENT);
                new_table->m_entries[i] = new_page;
            }
            pd_entry* new_entry = &(_current_dir->m_entries[i]);
            pd_entry_add_attrib(new_entry, I86_PDE_PRESENT | I86_PTE_WRITABLE);
            pd_entry_set_frame(new_entry, (uint32_t)new_table);
            return build_virt_addr(i, 0);
        }
    }
    return NULL;
}

void* vmm_alloc_blocks(size_t size) {
    uint32_t total_pages_needed = size / PAGE_SIZE + (size % PAGE_SIZE ? 1 : 0);
    uint32_t virtual_addr = vmm_find_next_free_s(total_pages_needed);
    if (virtual_addr == NULL) return NULL;
    uint32_t pd_index = PAGE_DIR_INDEX(virtual_addr);
    uint32_t pt_index = PAGE_TAB_INDEX(virtual_addr);
    pt_entry* page_table = pd_entry_pfn(_current_dir->m_entries[pd_index]);
    for (size_t i = 0; i/PAGE_SIZE < total_pages_needed; i+=PAGE_SIZE) {
        pt_entry_add_attrib(page_table + i + pt_index, I86_PTE_PRESENT | I86_PTE_WRITABLE);
    }
    return (void*)virtual_addr;
}

void vmm_free_block(uint32_t virtual_addr) {
    uint32_t pd_index = PAGE_DIR_INDEX(virtual_addr);
    uint32_t pt_index = PAGE_TAB_INDEX(virtual_addr);
    pt_entry* page_table = pd_entry_pfn(_current_dir->m_entries[pd_index]);
    vmm_free_page(page_table + pt_index * PAGE_SIZE);
}

extern uint32_t start_kernel_virt;
extern uint32_t start_kernel;

void vmm_init() {
	p_table* table = (p_table*)pmm_alloc_block();
	if (!table) return;
	p_table* table2 = (p_table*)pmm_alloc_block();
	if (!table2) return;
	memset(table, 0, sizeof(p_table));
	memset(table2, 0, sizeof(p_table));
	//! 1st 4mb are idenitity mapped
	for (int i=0, frame=0x0, virt=0x00000000; i<PAGES_PER_TABLE; i++, frame+=PAGE_SIZE, virt+=PAGE_SIZE) {

 		//! create a new page
		pt_entry page=0;
		pt_entry* page_addr = &page;
		pt_entry_add_attrib(page_addr, I86_PTE_PRESENT | I86_PTE_WRITABLE);
		pt_entry_set_frame (page_addr, frame);

		//! ...and add it to the page table
		table2->m_entries [PAGE_TAB_INDEX(virt) ] = page;
	}
		//! map 1mb to 3gb (where we are at)
	for (uint32_t i=0, frame=&start_kernel, virt=&start_kernel_virt; i<PAGES_PER_TABLE; i++, frame+=PAGE_SIZE, virt+=PAGE_SIZE) {

		//! create a new page
		pt_entry page=0;
		pt_entry* page_addr = &page;
		pt_entry_add_attrib(page_addr, I86_PTE_PRESENT | I86_PTE_WRITABLE);
		pt_entry_set_frame (page_addr, frame);

		//! ...and add it to the page table
		table->m_entries [PAGE_TAB_INDEX (virt) ] = page;
	}
		//! create default directory table
	p_dir*	dir = (p_dir*) pmm_alloc_blocks (3);
	if (!dir) return;
 
	//! clear directory table and set it as current
	memset (dir, 0, sizeof (p_dir));
	pd_entry* entry = &dir->m_entries [PAGE_DIR_INDEX ((uint32_t)&start_kernel_virt) ];
	pd_entry_add_attrib (entry, I86_PDE_PRESENT | I86_PDE_WRITABLE);
	pd_entry_set_frame (entry, (uint32_t)table);

	pd_entry* entry2 = &dir->m_entries [PAGE_DIR_INDEX (0x00000000) ];
	pd_entry_add_attrib (entry2, I86_PDE_PRESENT | I86_PDE_WRITABLE);
	pd_entry_set_frame (entry2, (uint32_t)table2);
		//! store current PDBR
	_cur_pdbr = (uint32_t) &dir->m_entries;

	//! switch to our page directory
	vmm_switch_pdir(dir);

	//enable_paging();
}
