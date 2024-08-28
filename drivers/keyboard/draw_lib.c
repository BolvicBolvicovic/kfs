#include "keyboard.h"

list_option_t draw_list(char* name, char** list, uint8_t row, uint8_t col, uint8_t span) {
    uint8_t* vga_memory = (uint8_t*)VGA_MEMORY;    
    size_t index = row * VGA_COLS + col;
    size_t name_len = strlen(name);
    if (name_len > VGA_COLS / 2)        { printf("Error: name of the list too long\n");  return (list_option_t) {.null = NULL}; }
    if (name_len + span > VGA_COLS / 2) { printf("Error: span of the list too long\n");  return (list_option_t) {.null = NULL}; }
    if (index > VGA_COLS * VGA_ROWS * 2)    { printf("Error: index of list out of bound\n"); return (list_option_t) {.null = NULL}; }
    size_t i;
    for (i = 0; i < name_len; i++) {
        vga_memory[index + (i * 2)] = name[i];
    }
    index += (i * 2) + span;
    for (i = 0; i < list[0][i]; i++) {
        vga_memory[index + (i * 2)] = list[0][i];
    }
    list_t _list = {.list = list, .current_item_index = 0, .list_vga_index = index - (i * 2)};
    return (list_option_t) {.list = _list};
}
