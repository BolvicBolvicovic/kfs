#include "pdt.h"

inline void pt_entry_add_attrib (pt_entry* e, uint32_t attrib) {
	*e |= attrib;
}

inline void pt_entry_del_attrib (pt_entry* e, uint32_t attrib) {
	*e &= ~attrib;
}

inline void pt_entry_set_frame (pt_entry* e, uint32_t addr) {
	*e = (*e & ~I86_PTE_FRAME) | addr;
}

inline int pt_entry_is_present (pt_entry e) {
	return e & I86_PTE_PRESENT;
}

inline int pt_entry_is_writable (pt_entry e) {
	return e & I86_PTE_WRITABLE;
}

inline uint32_t pt_entry_pfn (pt_entry e) {
	return e & I86_PTE_FRAME;
}

inline void pd_entry_add_attrib (pd_entry* e, uint32_t attrib) {
	*e |= attrib;
}

inline void pd_entry_del_attrib (pd_entry* e, uint32_t attrib) {
	*e &= ~attrib;
}

inline void pd_entry_set_frame (pd_entry* e, uint32_t addr) {
	*e = (*e & ~I86_PDE_FRAME) | addr;
}

inline int pd_entry_is_present (pd_entry e) {
	return e & I86_PDE_PRESENT;
}

inline int pd_entry_is_writable (pd_entry e) {
	return e & I86_PDE_WRITABLE;
}

inline uint32_t pd_entry_pfn (pd_entry e) {
	return e & I86_PDE_FRAME;
}

inline int pd_entry_is_user (pd_entry e) {
	return e & I86_PDE_USER;
}

inline int pd_entry_is_4mb (pd_entry e) {
	return e & I86_PDE_4MB;
}
