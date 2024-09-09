#ifndef PDT_H
#define PDT_H

#include <stdint.h>
// PAGE TABLE ENTRY 

enum PAGE_PTE_FLAGS {
								// Represents what bits of a 32 bits value
	I86_PTE_PRESENT			=	1,		// 0000000000000000000000000000001
	I86_PTE_WRITABLE		=	2,		// 0000000000000000000000000000010
	I86_PTE_USER			=	4,		// 0000000000000000000000000000100
	I86_PTE_WRITETHOUGH		=	8,		// 0000000000000000000000000001000
	I86_PTE_NOT_CACHEABLE		=	0x10,		// 0000000000000000000000000010000
	I86_PTE_ACCESSED		=	0x20,		// 0000000000000000000000000100000
	I86_PTE_DIRTY			=	0x40,		// 0000000000000000000000001000000
	I86_PTE_PAT			=	0x80,		// 0000000000000000000000010000000
	I86_PTE_CPU_GLOBAL		=	0x100,		// 0000000000000000000000100000000
	I86_PTE_LV4_GLOBAL		=	0x200,		// 0000000000000000000001000000000
   	I86_PTE_FRAME			=	0x7FFFF000 	// 1111111111111111111000000000000
};

typedef uint32_t pt_entry;

// PAGE DIRECTORY ENTRY

enum PAGE_PDE_FLAGS {
 
	I86_PDE_PRESENT			=	1,		//0000000000000000000000000000001
	I86_PDE_WRITABLE		=	2,		//0000000000000000000000000000010
	I86_PDE_USER			=	4,		//0000000000000000000000000000100
	I86_PDE_PWT			=	8,		//0000000000000000000000000001000
	I86_PDE_PCD			=	0x10,		//0000000000000000000000000010000
	I86_PDE_ACCESSED		=	0x20,		//0000000000000000000000000100000
	I86_PDE_DIRTY			=	0x40,		//0000000000000000000000001000000
	I86_PDE_4MB			=	0x80,		//0000000000000000000000010000000
	I86_PDE_CPU_GLOBAL		=	0x100,		//0000000000000000000000100000000
	I86_PDE_LV4_GLOBAL		=	0x200,		//0000000000000000000001000000000
   	I86_PDE_FRAME			=	0x7FFFF000 	//1111111111111111111000000000000
};
 
typedef uint32_t pd_entry;
//! sets a flag in the page table entry
extern void pd_entry_add_attrib (pd_entry* e, uint32_t attrib);

//! clears a flag in the page table entry
extern void pd_entry_del_attrib (pd_entry* e, uint32_t attrib);

//! sets a frame to page table entry
extern void pd_entry_set_frame (pd_entry*, uint32_t);

//! test if page is present
extern int pd_entry_is_present (pd_entry e);

//! test if directory is user mode
extern int pd_entry_is_user (pd_entry);

//! test if directory contains 4mb pages
extern int pd_entry_is_4mb (pd_entry);

//! test if page is writable
extern int pd_entry_is_writable (pd_entry e);

//! get page table entry frame address
extern uint32_t pd_entry_pfn (pd_entry e);

//! enable global pages
extern void pd_entry_enable_global (pd_entry e);

//! sets a flag in the page table entry
extern void pt_entry_add_attrib (pt_entry* e, uint32_t attrib);

//! clears a flag in the page table entry
extern void pt_entry_del_attrib (pt_entry* e, uint32_t attrib);

//! sets a frame to page table entry
extern void pt_entry_set_frame (pt_entry*, uint32_t);

//! test if page is present
extern int pt_entry_is_present (pt_entry e);

//! test if page is writable
extern int pt_entry_is_writable (pt_entry e);

//! get page table entry frame address
extern uint32_t pt_entry_pfn (pt_entry e);

#endif
