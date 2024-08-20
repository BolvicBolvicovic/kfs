#include "descriptor.h"

static gdt_register_t	gdt[5];
static gdt_register_t	gdt_reg;

extern void	gdt_flush(uint32_t);

static void	set_gdt_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
	gdt[num].base_low	= (base	& 0xFFFF);
	gdt[num].base_middle	= (base	>> 16) & 0xFF;
	gdt[num].base_high	= (base >> 24) & 0xFF;
	gdt[num].limit_low	= (limit & 0xFFFF);
	gdt[num].granularity	= (limit >> 16) & 0x0F;
	gdt[num].granularity	= gran & 0xF0;
	gdt[num].access		= access;
}

void	init_gdt() {
	//TODO
}
