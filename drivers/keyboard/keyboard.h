#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <stdint.h>
#include "../vga/vga.h"

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

#define LOW_16(addr)	(uint16_t)((addr) & 0xFFFF)
#define HIGH_16(addr)	(uint16_t)((addr >> 16) & 0xFFFF)

#endif
