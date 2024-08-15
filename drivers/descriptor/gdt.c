#include "descriptor.h"

static gdt_gate_t	gdt[5]__attribute__((section(".gdt"), aligned(8)));
static gdt_register_t	gdt_reg;

extern void	gdt_flush(uint32_t);

static void	set_gdt_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
	gdt[num].base_low	= (base	& 0xFFFF);
	gdt[num].base_middle= (base	>> 16) & 0xFF;
	gdt[num].base_high	= (base >> 24) & 0xFF;
	gdt[num].limit_low	= (limit & 0xFFFF);
	gdt[num].granularity= (limit >> 16) & 0x0F;
	gdt[num].granularity= gran & 0xF0;
	gdt[num].access		= access;
}

void	init_gdt() {
	gdt_reg.limit	= (sizeof(gdt_gate_t) * 5) - 1;
	gdt_reg.base	= (uint32_t)&gdt;

	set_gdt_gate(0, 0, 0, 0, 0);		    // NULL segment
	set_gdt_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
	set_gdt_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
	set_gdt_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
	set_gdt_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment

	gdt_flush((uint32_t)&gdt_reg);
}
