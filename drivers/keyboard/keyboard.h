#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <stdint.h>
#include "../vga/vga.h"
#include "../descriptor/descriptor.h"

void    init_keyboard();
void    exec_command();
void    cmd_add_char(uint8_t c);
void    tests_string(int* total, int* success, int* failure);
void    tests_stdlib(int* total, int* success, int* failure);

typedef enum {
    SHELL,
    SETTINGS
} screen_type_t;

typedef struct {
    char**  list;
    uint8_t current_item_index;
    size_t list_vga_index;
} list_t;

typedef union {
    list_t list;
    void* null;
} list_option_t;

typedef struct {
    screen_type_t type;
    list_option_t lists[3];
} current_screen_t;


void            init_current_screen(enum vga_color fg, enum vga_color bg);
list_option_t   draw_list(char* name, const char** list, uint8_t list_index, uint8_t row, uint8_t col, uint8_t span);
void            draw_line(char* line, uint8_t row, uint8_t col);
void            draw_selector(size_t index);
void            draw_name(char** list, size_t item_index, size_t vga_index);
void            clear_selector(size_t index);

#endif
