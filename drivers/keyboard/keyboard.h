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

typedef enum {
    SHELL,
    SETTINGS
} screen_type_t;

typedef struct {
    char**  list;
    uint8_t current_item_index;
    uint8_t list_vga_index;
} list_t;

typedef union {
    list_t list;
    void* null;
} list_option_t;

typedef struct {
    screen_type_t type;
    list_option_t lists[2];
} current_screen_t;


list_option_t draw_list(char* name, char** list, uint8_t row, uint8_t col, uint8_t span);

#endif
