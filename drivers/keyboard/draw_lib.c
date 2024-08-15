#include "keyboard.h"

extern current_screen_t current_screen;
extern const char*  color_list[16];

void    draw_name(char** list, size_t item_index, size_t vga_index) {
   uint8_t* vga_memory = (uint8_t*)VGA_MEMORY;    
   for (size_t i = 0; list[item_index][i]; i++) {
        vga_memory[vga_index] = list[item_index][i];
        vga_index += 2;
   }
   for (; vga_memory[vga_index] != ' '; vga_index += 2) {
        vga_memory[vga_index] = ' ';
   }
}

void    clear_selector(size_t index) {
   uint8_t new_color = term_get_color();
   uint8_t* vga_memory = (uint8_t*)VGA_MEMORY;    
   for (size_t i = index; vga_memory[i] != ' '; i+= 2) {
        vga_memory[i + 1] = new_color;
   }
}

void    draw_selector(size_t index) {
   enum vga_color bg = current_screen.lists[1].list.current_item_index;
   enum vga_color fg = current_screen.lists[0].list.current_item_index;
   uint8_t new_color = vga_entry_color(fg, bg);
   uint8_t* vga_memory = (uint8_t*)VGA_MEMORY;    
   for (size_t i = index; vga_memory[i] != ' '; i+= 2) {
        vga_memory[i + 1] = new_color;
   }
}

void    draw_line(char* line, uint8_t row, uint8_t col) {
    uint8_t* vga_memory = (uint8_t*)VGA_MEMORY;    
    size_t index = row * VGA_COLS + col;
    size_t line_len = strlen(line);
    if (line_len > VGA_COLS)        { printf("Error: line too long\n");  return; }
    for (size_t i = 0; i < line_len; i++) {
        vga_memory[index + (i * 2)] = line[i];
    }
}

list_option_t draw_list(char* name, char** list, uint8_t list_index, uint8_t row, uint8_t col, uint8_t span) {
    uint8_t* vga_memory = (uint8_t*)VGA_MEMORY;    
    size_t index = row * VGA_COLS + col;
    size_t name_len = strlen(name);
    if (name_len > VGA_COLS / 2)        { printf("Error: name of the list too long\n");  return (list_option_t) {.null = NULL}; }
    if (index > VGA_COLS * VGA_ROWS * 2)    { printf("Error: index of list out of bound\n"); return (list_option_t) {.null = NULL}; }
    size_t i;
    for (i = 0; i < name_len; i++) {
        vga_memory[index + (i * 2)] = name[i];
    }
    index += (i * 2) + span;
    size_t vga_index = index;
    for (i = 0; list[list_index][i]; i++) {
        vga_memory[index + (i * 2)] = list[list_index][i];
    }
    list_t _list = {.list = list, .current_item_index = list_index, .list_vga_index = vga_index};
    return (list_option_t) {.list = _list};
}
