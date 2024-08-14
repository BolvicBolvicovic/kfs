#ifndef CPU_H
#define CPU_H
#include <stdint.h>
#include "../utils.h"
#include "../vga/vga.h"

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
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

typedef struct {
	uint32_t	ds;
	uint32_t	edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t	int_no, err_code;
	uint32_t	eip, cs, eflafs, useresp, ss;
} registers_t;

typedef struct {
	uint16_t	low_offset;
	uint16_t	selector;
	uint16_t	always0;
	uint16_t	flags;
	uint16_t	high_offset;
} __attribute__((packed)) idt_gate_t;

typedef struct {
    uint16_t    limit;
    uint32_t    base;
} __attribute__((packed)) idt_register_t;

typedef void (*isr_t)(registers_t *);

#define LOW_16(addr)	(uint16_t)((addr) & 0xFFFF)
#define HIGH_16(addr)	(uint16_t)((addr >> 16) & 0xFFFF)

void	set_idt_gate(int n, uint32_t handler);
void    isr_handler(registers_t *r);
void    irq_handler(registers_t *r);
void    load_idt();
void    register_interrupt_handler(uint8_t n, isr_t handler);
void    isr_install();

#endif
