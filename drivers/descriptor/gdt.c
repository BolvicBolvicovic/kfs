#include "descriptor.h"

#define SEGMENT_LIMIT 0xFFFFFFFF
#define SEGMENT_BASE 0
/***	 gdt descriptor access bit flags.	***/

//! set access bit
#define I86_GDT_DESC_ACCESS			0x0001			//00000001

//! descriptor is readable and writable. default: read only
#define I86_GDT_DESC_READWRITE			0x0002			//00000010

//! set expansion direction bit
#define I86_GDT_DESC_EXPANSION			0x0004			//00000100

//! executable code segment. Default: data segment
#define I86_GDT_DESC_EXEC_CODE			0x0008			//00001000

//! set code or data descriptor. defult: system defined descriptor
#define I86_GDT_DESC_CODEDATA			0x0010			//00010000

//! set dpl bits
#define I86_GDT_DESC_DPL			0x0060			//01100000

//! set "in memory" bit
#define I86_GDT_DESC_MEMORY			0x0080			//10000000

/**	gdt descriptor grandularity bit flags	***/

//! masks out limitHi (High 4 bits of limit)
#define I86_GDT_GRAND_LIMITHI_MASK		0x0f			//00001111

//! set os defined bit
#define I86_GDT_GRAND_OS			0x10			//00010000

//! set if 32bit. default: 16 bit
#define I86_GDT_GRAND_32BIT			0x40			//01000000

//! 4k grandularity. default: none
#define I86_GDT_GRAND_4K			0x80			//10000000

static gdt_gate_t	gdt[5];//__attribute__((section(".gdt"), aligned(8)));
static gdt_register_t	gdt_reg;

extern void	gdt_flush(uint32_t);

static void	set_gdt_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
	// Set Adresses
	gdt[num].base_low	= (base	& 0xFFFF);
	gdt[num].base_middle    = (base	>> 16) & 0xFF;
	gdt[num].base_high	= (base >> 24) & 0xFF;
	gdt[num].limit_low	= (limit & 0xFFFF);

	// Set Flags
	gdt[num].granularity    = (limit >> 16) & 0x0F;
	gdt[num].granularity   |= gran & 0xF0;
	gdt[num].access		= access;
}

void	init_gdt() {
	gdt_reg.limit	= (sizeof(gdt_gate_t) * 5) - 1;
	gdt_reg.base	= (uint32_t)&gdt;

	set_gdt_gate(0, 0, 0, 0, 0);		    // NULL segment
	set_gdt_gate(1, SEGMENT_BASE, SEGMENT_LIMIT,
	      I86_GDT_DESC_READWRITE | I86_GDT_DESC_EXEC_CODE | I86_GDT_DESC_CODEDATA | I86_GDT_DESC_MEMORY,
	      I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK 
	); // Code segment
	set_gdt_gate(2, SEGMENT_BASE, SEGMENT_LIMIT,
	      I86_GDT_DESC_READWRITE | I86_GDT_DESC_CODEDATA | I86_GDT_DESC_MEMORY,
	      I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK 
	); // Data segment
	set_gdt_gate(3, SEGMENT_BASE, SEGMENT_LIMIT,
	      I86_GDT_DESC_DPL | I86_GDT_DESC_READWRITE | I86_GDT_DESC_EXEC_CODE | I86_GDT_DESC_CODEDATA | I86_GDT_DESC_MEMORY,
	      I86_GDT_DESC_DPL | I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK 
	); // User mode code segment
	set_gdt_gate(4, SEGMENT_BASE, SEGMENT_LIMIT,
	      I86_GDT_DESC_DPL | I86_GDT_DESC_READWRITE | I86_GDT_DESC_CODEDATA | I86_GDT_DESC_MEMORY,
	      I86_GDT_DESC_DPL | I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK 
	); // User mode data segment

	gdt_flush((uint32_t)&gdt_reg);
}
