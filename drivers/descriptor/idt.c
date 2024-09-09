#include "descriptor.h"

// INTERUPTION DESCRIPTOR TABLE

#define ICW1 0x11			    // For ICW1 : 0001 0001
					    // 0001 0000 bit tells the PIC that this is Initialization Command 
					    // 0000 0001 bit tells the PIC that we'll need X86_MODE (ICW4)
#define MASTER_IDT_OFFSET 0x20
#define SLAVE_IDT_OFFSET 0x28
#define SLAVE_CONNECTION_LINE_FOR_M 0x04
#define SLAVE_CONNECTION_LINE_FOR_S 0x02
#define X86_MODE 1
#define ENABLE_INTERRUPT 0

static idt_gate_t	    idt[256] = {0};
static idt_register_t   idt_reg;

void    init_idt() {
    idt_reg.limit = sizeof(idt_gate_t) * 256 -1;
    idt_reg.base  = (uint32_t)&idt;

    set_idt_gate(0, (uint32_t) isr0);
    set_idt_gate(1, (uint32_t) isr1);
    set_idt_gate(2, (uint32_t) isr2);
    set_idt_gate(3, (uint32_t) isr3);
    set_idt_gate(4, (uint32_t) isr4);
    set_idt_gate(5, (uint32_t) isr5);
    set_idt_gate(6, (uint32_t) isr6);
    set_idt_gate(7, (uint32_t) isr7);
    set_idt_gate(8, (uint32_t) isr8);
    set_idt_gate(9, (uint32_t) isr9);
    set_idt_gate(10, (uint32_t) isr10);
    set_idt_gate(11, (uint32_t) isr11);
    set_idt_gate(12, (uint32_t) isr12);
    set_idt_gate(13, (uint32_t) isr13);
    set_idt_gate(14, (uint32_t) isr14);
    set_idt_gate(15, (uint32_t) isr15);
    set_idt_gate(16, (uint32_t) isr16);
    set_idt_gate(17, (uint32_t) isr17);
    set_idt_gate(18, (uint32_t) isr18);
    set_idt_gate(19, (uint32_t) isr19);
    set_idt_gate(20, (uint32_t) isr20);
    set_idt_gate(21, (uint32_t) isr21);
    set_idt_gate(22, (uint32_t) isr22);
    set_idt_gate(23, (uint32_t) isr23);
    set_idt_gate(24, (uint32_t) isr24);
    set_idt_gate(25, (uint32_t) isr25);
    set_idt_gate(26, (uint32_t) isr26);
    set_idt_gate(27, (uint32_t) isr27);
    set_idt_gate(28, (uint32_t) isr28);
    set_idt_gate(29, (uint32_t) isr29);
    set_idt_gate(30, (uint32_t) isr30);
    set_idt_gate(31, (uint32_t) isr31);

    // PIC1 is 0x21 and PIC2 is 0xA1 (PIC = Programable Interrupt Controller)
    // We send them Initialization Command Words (ICW)

    // ICW1
    port_byte_out(CMD_MASTER_PORT, ICW1);
    port_byte_out(CMD_SLAVE_PORT, ICW1);
    // ICW2
    port_byte_out(DATA_MASTER_PORT, MASTER_IDT_OFFSET);
    port_byte_out(DATA_SLAVE_PORT, SLAVE_IDT_OFFSET);

    // ICW3
    port_byte_out(DATA_MASTER_PORT, SLAVE_CONNECTION_LINE_FOR_M);
    port_byte_out(DATA_SLAVE_PORT, SLAVE_CONNECTION_LINE_FOR_S);

    // ICW4
    port_byte_out(DATA_MASTER_PORT, X86_MODE);
    port_byte_out(DATA_SLAVE_PORT, X86_MODE);

    // OCW1 (Operational Command Word) It enables all IRCs.
    port_byte_out(DATA_MASTER_PORT, ENABLE_INTERRUPT);
    port_byte_out(DATA_SLAVE_PORT, ENABLE_INTERRUPT);

    // Install the IRQs
    set_idt_gate(IRQ0, (uint32_t)irq0);
    set_idt_gate(IRQ1, (uint32_t)irq1);
    set_idt_gate(IRQ2, (uint32_t)irq2);
    set_idt_gate(IRQ3, (uint32_t)irq3);
    set_idt_gate(IRQ4, (uint32_t)irq4);
    set_idt_gate(IRQ5, (uint32_t)irq5);
    set_idt_gate(IRQ6, (uint32_t)irq6);
    set_idt_gate(IRQ7, (uint32_t)irq7);
    set_idt_gate(IRQ8, (uint32_t)irq8);
    set_idt_gate(IRQ9, (uint32_t)irq9);
    set_idt_gate(IRQ10, (uint32_t)irq10);
    set_idt_gate(IRQ11, (uint32_t)irq11);
    set_idt_gate(IRQ12, (uint32_t)irq12);
    set_idt_gate(IRQ13, (uint32_t)irq13);
    set_idt_gate(IRQ14, (uint32_t)irq14);
    set_idt_gate(IRQ15, (uint32_t)irq15);


    // Syscall

    set_idt_gate(SYSCALL, (uint32_t)syscall);

    asm volatile("lidt (%0)" : : "r" (&idt_reg));
}

void	set_idt_gate(int n, uint32_t handler) {
    idt[n].base_low = LOW_16(handler);
    idt[n].selector = 0x08; // GDT address
    idt[n].always0 = 0;
    idt[n].flags = 0x8E;
    // 0x8E = 1  00 0 1  110
    //        P DPL 0 D Type
    // In user-mode, we should uncomment the OR bellow.
    // It sets interrupt gate's privilege to lvl 3.
    idt[n].base_high = HIGH_16(handler)/* | 0x60 */;
}
