#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <stdint.h>
#include "../vga/vga.h"
#include "../descriptor/descriptor.h"

void    init_keyboard();
void    exec_command();
void    cmd_add_char(uint8_t c);

#endif
