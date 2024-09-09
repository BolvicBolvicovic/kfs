// Programmable Interval Timer

#ifndef PIT_T
#define PIT_T
#include "../descriptor/descriptor.h"

#define CLOCK_RATE 1193180
#define PIT_CTRL_WORD 0x43
#define PIT_COUNTER_0 0x40

void init_timer(uint32_t frequency);

#endif
