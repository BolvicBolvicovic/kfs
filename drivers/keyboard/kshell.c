#include "keyboard.h"

static char line[256];
static size_t index = 0;

static inline void set_keyboard(char* layout) {
    if (!strcmp(layout, "US\0")) set_keyboard_index(0);
    else if (!strcmp(layout, "FR\0")) set_keyboard_index(1);
    else {
        printf("USAGE: SET KEYBOARD [LAYOUT]\n");
    }
}

static inline void set_color(char* foreground, char* background) {
    enum vga_color f = 0;
    enum vga_color b = 0;
    if (!strcmp(foreground, "BLACK\0")) f = BLACK; 
    else if (!strcmp(foreground, "BLUE\0")) f = BLUE; 
    else if (!strcmp(foreground, "GREEN\0")) f = GREEN; 
    else if (!strcmp(foreground, "CYAN\0")) f = CYAN; 
    else if (!strcmp(foreground, "RED\0")) f = RED; 
    else if (!strcmp(foreground, "MAGENTA\0")) f = MAGENTA; 
    else if (!strcmp(foreground, "BROWN\0")) f = BROWN; 
    else if (!strcmp(foreground, "LIGHT_GRAY\0")) f = LIGHT_GRAY; 
    else if (!strcmp(foreground, "DARK_GREY\0")) f = DARK_GREY; 
    else if (!strcmp(foreground, "LIGHT_BLUE\0")) f = LIGHT_BLUE; 
    else if (!strcmp(foreground, "LIGHT_GREEN\0")) f = LIGHT_GREEN; 
    else if (!strcmp(foreground, "LIGHT_RED\0")) f = LIGHT_RED; 
    else if (!strcmp(foreground, "LIGHT_CYAN\0")) f = LIGHT_CYAN; 
    else if (!strcmp(foreground, "PINK\0")) f = PINK; 
    else if (!strcmp(foreground, "YELLOW\0")) f = YELLOW; 
    else if (!strcmp(foreground, "WHITE\0")) f = WHITE; 
    else { printf("USAGE: SET COLOR [FOREGROUND] [BACKGROUND]"); return; }
    if (!strcmp(background, "BLACK\0")) b = BLACK; 
    else if (!strcmp(background, "BLUE\0")) b = BLUE; 
    else if (!strcmp(background, "GREEN\0")) b = GREEN; 
    else if (!strcmp(background, "CYAN\0")) b = CYAN; 
    else if (!strcmp(background, "RED\0")) b = RED; 
    else if (!strcmp(background, "MAGENTA\0")) b = MAGENTA; 
    else if (!strcmp(background, "BROWN\0")) b = BROWN; 
    else if (!strcmp(background, "LIGHT_GRAY\0")) b = LIGHT_GRAY; 
    else if (!strcmp(background, "DARK_GREY\0")) b = DARK_GREY; 
    else if (!strcmp(background, "LIGHT_BLUE\0")) b = LIGHT_BLUE; 
    else if (!strcmp(background, "LIGHT_GREEN\0")) b = LIGHT_GREEN; 
    else if (!strcmp(background, "LIGHT_RED\0")) b = LIGHT_RED; 
    else if (!strcmp(background, "LIGHT_CYAN\0")) b = LIGHT_CYAN; 
    else if (!strcmp(background, "PINK\0")) b = PINK; 
    else if (!strcmp(background, "YELLOW\0")) b = YELLOW; 
    else if (!strcmp(background, "WHITE\0")) b = WHITE; 
    else { printf("USAGE: SET COLOR [FOREGROUND] [BACKGROUND]"); return; }
    term_set_color(vga_entry_color(f, b));
    term_clear();
}

inline void cmd_add_char(uint8_t c) {
    if (c == 0x7F && index - 1 >= 0) index--;
    else if (index < 256) line[index++] = c;
}

void    exec_command() {
    char words[4][256];
    size_t i = 0;
    size_t j;

    line[index] = 0;
    index = 0;

    while (line[i] == ' ') i++;
    for (size_t k = 0; k < 4; k++) {
        for (j = 0; line[i] >= 0x21 && line[i] <= 0x7E; j++) words[k][j] = line[i++];
        words[k][j] = 0;
    }
    if (strcmp(words[0], "SET\0")) {
        if (!strcmp(words[1], "KEYBOARD\0")) set_keyboard(words[2]);
        else if (!strcmp(words[2], "COLOR\0")) set_color(words[2], words[3]);
    }
}
