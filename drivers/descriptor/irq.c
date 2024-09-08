#include "descriptor.h"

static isr_t interrupt_handlers[256] = {0};

void    irq_handler(registers_t* r) {
    if (r->int_no >= FIRST_SLAVE_PORT) {
        port_byte_out(SLAVE_PORT, EOI);
    }
    port_byte_out(MASTER_PORT, EOI);
    if (interrupt_handlers[r->int_no]) {
        isr_t handler = interrupt_handlers[r->int_no];
        handler(r);
    }
}

void    register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}
