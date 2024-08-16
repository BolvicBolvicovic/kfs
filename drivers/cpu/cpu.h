#ifndef CPU_H
#define CPU_H
#include <stdint.h>
#include "../utils.h"
#include "../vga/vga.h"

typedef struct {
	uint32_t	ds;
	uint32_t	edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t	int_no, err_code;
	uint32_t	eip, cs, eflafs, useresp, ss;
} registers_t;

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

typedef struct {
	uint16_t	low_offset;
	uint16_t	selector;
	uint16_t	always0;
	uint16_t	flags;
	uint16_t	high_offset;
} __attribute__((packed)) idt_gate_t;

#define LOW_16(addr)	(uint16_t)((addr) & 0xFFFF)
#define HIGH_16(addr)	(uint16_t)((addr >> 16) & 0xFFFF)



#endif
