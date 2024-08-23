#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <stdint.h>
#include "../vga/vga.h"
#include "../descriptor/descriptor.h"

void    init_keyboard();
void    set_keyboard_index(size_t i);
void    exec_command();
void    cmd_add_char(uint8_t c);
void    tests_string(int* total, int* success, int* failure);

#endif
