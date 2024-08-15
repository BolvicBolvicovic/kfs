#include "descriptor.h"

static isr_t interrupt_handlers[256];

void    irq_handler(registers_t *r) {
    if (interrupt_handlers[r->int_no]) {
        isr_t handler = interrupt_handlers[r->int_no];
        handler(r);
    }
    port_byte_out(0x20, 0x20); // primary end of interrupt (EOI)
    if (r->int_no < 40) {
        port_byte_out(0xA0, 0x20); // secondary EOI
    }
}

void    register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}
