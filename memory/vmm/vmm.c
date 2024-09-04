#include "vmm.h"

p_dir*   _current_dir = NULL;
uint32_t _cur_pdbr = 0;

int vmm_alloc_page(pt_entry* entry) {
	void* p = pmm_alloc_block();
	if (!p) return 0;
	PT_ENTRY_SET_FRAME(entry, p);
	PT_ENTRY_ADD_ATTRIB(entry, I86_PTE_PRESENT);
	return 1;
}

void vmm_free_page(pt_entry* entry) {
	void* p = (void*)PT_ENTRY_PFN(entry);
	if (p) pmm_free_block(p);
	PT_ENTRY_ADD_ATTRIB(entry, I86_PTE_PRESENT);
}

inline pt_entry* vmm_ptable_lookup_entry(p_table* p, uint32_t addr) {
	if (p) return &p->m_entries[PAGE_TAB_INDEX(addr)];
	return 0;
}

inline pd_entry* vmm_pdir_lookup_entry(p_dir* p, uint32_t addr) {
	if (p) return &p->m_entries[PAGE_TAB_INDEX(addr)];
	return 0;
}

inline int vmm_switch_pdir(p_dir* dir) {
	if (!dir) return 0;
	_current_dir = dir;
	asm volatile("mov (%0), %%eax" : : "r" (_cur_pdbr));    // Loads _cur_pdbr
	asm volatile("mov %eax, %cr3");			// Sets cr3 with _cur_pdbr
	return 1;
}

void vmm_flush_tlb_entry(uint32_t addr) {
	asm volatile("cli");
	asm volatile("invlpg (%0)" :: "r" (addr)); // Invalid TLB (cache) entry. Used by supervisor only.
	asm volatile("sti");
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
		PD_ENTRY_ADD_ATTRIB(entry, I86_PDE_PRESENT);
		PD_ENTRY_ADD_ATTRIB(entry, I86_PDE_WRITABLE);
		PD_ENTRY_SET_FRAME(entry, (uint32_t)table);
	}
	p_table* table = (p_table*)PAGE_PHYS_ADDR(e);
	pt_entry* page = &table->m_entries[PAGE_TAB_INDEX((uint32_t)virt)];
	PT_ENTRY_SET_FRAME(page, (uint32_t)phys);
	PT_ENTRY_ADD_ATTRIB(page, I86_PTE_PRESENT);
}

void vmm_init() {
	p_table* table = (p_table*)pmm_alloc_block();
	if (!table) return;
	p_table* table2 = (p_table*)pmm_alloc_block();
	if (!table2) return;
    memset(table, 0, sizeof(p_table));
	//! 1st 4mb are idenitity mapped
	for (int i=0, frame=0x0, virt=0x00000000; i<1024; i++, frame+=4096, virt+=4096) {

 		//! create a new page
		pt_entry page=0;
        pt_entry* page_addr = &page;
		PT_ENTRY_ADD_ATTRIB(page_addr, I86_PTE_PRESENT);
		PT_ENTRY_SET_FRAME (page_addr, frame);

		//! ...and add it to the page table
		table2->m_entries [PAGE_TAB_INDEX(virt) ] = page;
	}
		//! map 1mb to 3gb (where we are at)
	for (int i=0, frame=0x100000, virt=0xc0000000; i<1024; i++, frame+=4096, virt+=4096) {

		//! create a new page
		pt_entry page=0;
        pt_entry* page_addr = &page;
		PT_ENTRY_ADD_ATTRIB(page_addr, I86_PTE_PRESENT);
		PT_ENTRY_SET_FRAME (page_addr, frame);

		//! ...and add it to the page table
		table->m_entries [PAGE_TAB_INDEX (virt) ] = page;
	}
		//! create default directory table
	p_dir*	dir = (p_dir*) pmm_alloc_blocks (3);
	if (!dir) return;
 
	//! clear directory table and set it as current
	memset (dir, 0, sizeof (p_dir));
	pd_entry* entry = &dir->m_entries [PAGE_DIR_INDEX (0xc0000000) ];
	PD_ENTRY_ADD_ATTRIB (entry, I86_PDE_PRESENT);
	PD_ENTRY_ADD_ATTRIB (entry, I86_PDE_WRITABLE);
	PD_ENTRY_SET_FRAME (entry, table);

	pd_entry* entry2 = &dir->m_entries [PAGE_DIR_INDEX (0x00000000) ];
	PD_ENTRY_ADD_ATTRIB (entry2, I86_PDE_PRESENT);
	PD_ENTRY_ADD_ATTRIB (entry2, I86_PDE_WRITABLE);
	PD_ENTRY_SET_FRAME (entry2, table2);
		//! store current PDBR
	_cur_pdbr = (uint32_t) &dir->m_entries;
 
	//! switch to our page directory
	vmm_switch_pdir(dir);
 
	//! enable paging
    asm volatile("mov %cr0, %eax\n\r");
    asm volatile("or  0x80000001, %eax\n\r");
    asm volatile("mov %eax, %cr0\n\r");
}
