#include "cpu.h"

//INTERUPTION DESCRIPTOR TABLE

static idt_gate_t	idt[256];

void	set_idt_gate(int n, uint32_t handler) {
	idt[n].low_offset = LOW_16(handler);
	idt[n].selector = 0x08;
	idt[n].always0 = 0;
	idt[n].flags = 0x8E;
	// 0x8E = 1  00 0 1  110
    //        P DPL 0 D Type
	idt[n].high_offset = HIGH_16(handler);
}
