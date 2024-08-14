#include "cpu.h"

//INTERUPTION DESCRIPTOR TABLE

static idt_gate_t	    idt[256];
static idt_register_t   idt_reg;

void	set_idt_gate(int n, uint32_t handler) {
	idt[n].low_offset = LOW_16(handler);
	idt[n].selector = 0x08;
	idt[n].always0 = 0;
	idt[n].flags = 0x8E;
	// 0x8E = 1  00 0 1  110
    //        P DPL 0 D Type
	idt[n].high_offset = HIGH_16(handler);
}

void    load_idt() {
    idt_reg.base = (uint32_t) &idt;
    idt_reg.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;
    asm volatile("lidt (%0)" : : "r" (&idt_reg)); //lidt = load interrupt descriptor table
}
