#include "pit.h"

static uint32_t tick = 0;

void	sleep(uint32_t ticks) {
	uint32_t _tick = tick;
	while (tick > _tick && tick - _tick < ticks) continue;
}

static void timer_callback(registers_t* regs) {
	tick++;
	return;
}

void init_timer(uint32_t frequency) {
	register_interrupt_handler(IRQ0, &timer_callback);
	uint32_t divisor = CLOCK_RATE / frequency;
	//    00                 11                      011                         0
	// Counter 0 | RD or LD LSB then MSB | Mode 3: Square Wave Generator | Binary counter
	port_byte_out(PIT_CTRL_WORD, 0b110110);
	uint8_t lsb = (uint8_t)(divisor & 0xFF);
	uint8_t msb = (uint8_t)((divisor >> 8) & 0xFF);
	port_byte_out(PIT_COUNTER_0, lsb);
	port_byte_out(PIT_COUNTER_0, msb);
}
