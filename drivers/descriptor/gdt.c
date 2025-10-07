#include "descriptor.h"

extern void		gdt_flush(uint32_t);

#define SEGMENT_BASE				0
#define SEGMENT_LIMIT				0xFFFFFFFF

static gdt_gate_t		gdt[6];
static gdt_register_t	gdt_reg;

void
set_gdt_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
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

void
init_gdt()
{
	gdt_reg.limit	= (sizeof(gdt_gate_t) * 6) - 1;
	gdt_reg.base	= (uint32_t)&gdt;
	
	// Note: NULL segment
	set_gdt_gate(0, 0, 0, 0, 0);

	// Note: Code segment
	set_gdt_gate(1, SEGMENT_BASE, SEGMENT_LIMIT,
	      I86_GDT_DESC_READWRITE | I86_GDT_DESC_EXEC_CODE | I86_GDT_DESC_CODEDATA | I86_GDT_DESC_MEMORY,
	      I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);

	// Note: Data segment
	set_gdt_gate(2, SEGMENT_BASE, SEGMENT_LIMIT,
	      I86_GDT_DESC_READWRITE | I86_GDT_DESC_CODEDATA | I86_GDT_DESC_MEMORY,
	      I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);

	// Note: User mode code segment
	set_gdt_gate(3, SEGMENT_BASE, SEGMENT_LIMIT,
	      I86_GDT_DESC_DPL | I86_GDT_DESC_READWRITE | I86_GDT_DESC_EXEC_CODE | I86_GDT_DESC_CODEDATA | I86_GDT_DESC_MEMORY,
	      I86_GDT_DESC_DPL | I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);

	// Note: User mode data segment
	set_gdt_gate(4, SEGMENT_BASE, SEGMENT_LIMIT,
	      I86_GDT_DESC_DPL | I86_GDT_DESC_READWRITE | I86_GDT_DESC_CODEDATA | I86_GDT_DESC_MEMORY,
	      I86_GDT_DESC_DPL | I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);
	
	gdt_flush((uint32_t)&gdt_reg);
}
