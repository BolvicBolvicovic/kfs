#ifndef CPU_H
#define CPU_H
#include <stdint.h>
#include "../utils.h"
#include "../../lib/stdlib/stdlib.h"
#include "../../lib/stdio/stdio.h"
#include "../../lib/string/string.h"
#include "../../memory/pmm/pmm.h"

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
extern void syscall();

typedef struct {
	uint16_t	limit_low;
	uint16_t	base_low;
	uint8_t		base_middle;
	uint8_t		access;
	uint8_t		granularity;
	uint8_t		base_high;

} __attribute__((packed)) gdt_gate_t;

typedef struct {
	uint16_t	limit;
	uint32_t	base;
} __attribute__((packed)) gdt_register_t;

typedef struct {
	uint32_t	ds;
	uint32_t	edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t	int_no, err_code;
	uint32_t	eip, cs, eflafs, useresp, ss;
} registers_t;

typedef void (*isr_t)(registers_t*);

typedef struct {
	uint16_t	base_low;
	uint16_t	selector;
	uint8_t	    always0;
	uint8_t 	flags;
	uint16_t	base_high;
} __attribute__((packed)) idt_gate_t;

typedef struct {
    uint16_t    limit;
    uint32_t    base;
} __attribute__((packed)) idt_register_t;


#define LOW_16(addr)	(uint16_t)((addr) & 0xFFFF)
#define HIGH_16(addr)	(uint16_t)((addr >> 16) & 0xFFFF)

#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47
#define SYSCALL 0x80

#define SLAVE_PORT 0xA0
#define MASTER_PORT 0x20
#define EOI 0x20 // End Of Interrupt
#define FIRST_SLAVE_PORT 40

void	init_gdt();
void    init_idt();
void    isr_install();
void	set_idt_gate(int n, uint32_t handler);
void    isr_handler(registers_t* r);
void    irq_handler(registers_t* r);
void    register_interrupt_handler(uint8_t n, isr_t handler);

#endif
