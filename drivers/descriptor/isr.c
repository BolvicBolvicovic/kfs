#include "descriptor.h"

//INTERUPTION SERVICE ROUTINE

static char*		exception_msg[] = {
	"Division by zero",
	"Debug",
	"Non Maskable Interrupt",
	"Breakpoint",
	"Into Detected Overflow",
	"Out of Bounds",
	"Invalid Opcode",
	"No Coprocessor",

	"Double Fault",
	"Coprocessor Segment Overrun",
	"Bad TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"Unknown Interrupt",

	"Coprocessor Fault",
	"Alignment Check",
	"Machine Check",
	"Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
	"Reserved"
};

void	isr_handler(registers_t *r) {
	printf("%s\n", exception_msg[r->int_no]);
}

void isr_install() {

    init_idt();
	// PIC1 is 0x21 and PIC2 is 0xA1 (PIC = Programable Interrupt Controller)
	// We send them Initialization Command Words (ICW)

	// ICW1
    port_byte_out(0x20, 0x11);
    port_byte_out(0xA0, 0x11);

    // ICW2
    port_byte_out(0x21, 0x20);
    port_byte_out(0xA1, 0x28);

    // ICW3
    port_byte_out(0x21, 0x04);
    port_byte_out(0xA1, 0x02);

    // ICW4
    port_byte_out(0x21, 0x01);
    port_byte_out(0xA1, 0x01);

    // OCW1 (Operational Command Word) It enables all IRCs.
    port_byte_out(0x21, 0x0);
    port_byte_out(0xA1, 0x0);

        // Install the IRQs
    set_idt_gate(32, (uint32_t)irq0);
    set_idt_gate(33, (uint32_t)irq1);
    set_idt_gate(34, (uint32_t)irq2);
    set_idt_gate(35, (uint32_t)irq3);
    set_idt_gate(36, (uint32_t)irq4);
    set_idt_gate(37, (uint32_t)irq5);
    set_idt_gate(38, (uint32_t)irq6);
    set_idt_gate(39, (uint32_t)irq7);
    set_idt_gate(40, (uint32_t)irq8);
    set_idt_gate(41, (uint32_t)irq9);
    set_idt_gate(42, (uint32_t)irq10);
    set_idt_gate(43, (uint32_t)irq11);
    set_idt_gate(44, (uint32_t)irq12);
    set_idt_gate(45, (uint32_t)irq13);
    set_idt_gate(46, (uint32_t)irq14);
    set_idt_gate(47, (uint32_t)irq15);

    load_idt(); // Load with ASM
}
