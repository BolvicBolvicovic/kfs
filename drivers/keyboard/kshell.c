#include "keyboard.h"

static char line[256];
static size_t index = 0;
extern current_screen_t current_screen;

static inline void set_keyboard(char* layout) {
    if (!strcmp(layout, "US")) set_keyboard_index(0);
    else if (!strcmp(layout, "FR")) set_keyboard_index(1);
    else {
        printf("USAGE: SET KEYBOARD [LAYOUT]\n");
    }
}

static const char*  color_list[16] = {
	"BLACK",
	"BLUE",
	"GREEN",
	"CYAN",
	"RED",
	"MAGENTA",
	"BROWN",
	"WHITE",
	"GREY",
	"LIGHT_BLUE",
	"LIGHT_GREEN",
	"LIGHT_CYAN",
	"LIGHT_RED",
	"PINK",
	"YELLOW",
	"BRIGHT_WHITE"
};

static void set_color() {
    term_clear();
    list_option_t background = draw_list("BACKGROUND", color_list, (VGA_ROWS / 4) * 2, 10, 30);
    list_option_t foreground = draw_list("FORGROUND", color_list, (VGA_ROWS / 4) * 6, 10, 30);
    if (background.null == NULL || foreground.null == NULL) return;
    current_screen.type = SETTINGS;
    current_screen.lists[0] = background;
    current_screen.lists[1] = foreground;
}
/*
static void set_color(char* foreground, char* background) {
    enum vga_color f = -1;
    enum vga_color b = -1;
    if (!strcmp(foreground, "BLACK"))
        f = BLACK;
    else if (!strcmp(foreground, "BLUE"))
        f = BLUE; 
    else if (!strcmp(foreground, "GREEN"))
        f = GREEN; 
    else if (!strcmp(foreground, "CYAN"))
        f = CYAN; 
    else if (!strcmp(foreground, "RED"))
        f = RED; 
    else if (!strcmp(foreground, "MAGENTA"))
        f = MAGENTA; 
    else if (!strcmp(foreground, "BROWN"))
        f = BROWN; 
    else if (!strcmp(foreground, "WHITE"))
        f = WHITE; 
    else if (!strcmp(foreground, "GREY"))
        f = GREY; 
    else if (!strcmp(foreground, "LIGHT_BLUE"))
        f = LIGHT_BLUE; 
    else if (!strcmp(foreground, "LIGHT_GREEN"))
        f = LIGHT_GREEN; 
    else if (!strcmp(foreground, "LIGHT_RED"))
        f = LIGHT_RED; 
    else if (!strcmp(foreground, "LIGHT_CYAN"))
        f = LIGHT_CYAN; 
    else if (!strcmp(foreground, "PINK"))
        f = PINK; 
    else if (!strcmp(foreground, "YELLOW"))
        f = YELLOW; 
    else if (!strcmp(foreground, "BRIGHT_WHITE"))
        f = BRIGHT_WHITE; 
    else { printf("USAGE: SET COLOR [FOREGROUND] [BACKGROUND]\n"); return; }
    if (!strcmp(background, "BLACK"))
        b = BLACK; 
    else if (!strcmp(background, "BLUE"))
        b = BLUE; 
    else if (!strcmp(background, "GREEN"))
        b = GREEN; 
    else if (!strcmp(background, "CYAN"))
        b = CYAN; 
    else if (!strcmp(background, "RED"))
        b = RED; 
    else if (!strcmp(background, "MAGENTA"))
        b = MAGENTA; 
    else if (!strcmp(background, "BROWN"))
        b = BROWN; 
    else if (!strcmp(background, "WHITE"))
        b = WHITE;
    else if (!strcmp(background, "GREY"))
        b = GREY; 
    else if (!strcmp(background, "LIGHT_BLUE"))
        b = LIGHT_BLUE; 
    else if (!strcmp(background, "LIGHT_GREEN"))
        b = LIGHT_GREEN; 
    else if (!strcmp(background, "LIGHT_RED"))
        b = LIGHT_RED; 
    else if (!strcmp(background, "LIGHT_CYAN"))
        b = LIGHT_CYAN; 
    else if (!strcmp(background, "PINK"))
        b = PINK; 
    else if (!strcmp(background, "YELLOW"))
        b = YELLOW; 
    else if (!strcmp(background, "BRIGHT_WHITE"))
        b = BRIGHT_WHITE; 
    else { printf("USAGE: SET COLOR [FOREGROUND] [BACKGROUND]\n"); return; }
    term_set_color(vga_entry_color(f, b));
    term_clear();
}
*/
inline void cmd_add_char(uint8_t c) {
    if (c == 0x7F && index - 1 >= 0) index--;
    else if (index < 256) line[index++] = c;
    if (index >= VGA_COLS) index = 0;
}

void exec_tests() {
    int total = 0;
    int success = 0;
    int failure = 0;
    printf("TEST SUITE FOR THE KERNEL\n");
    //TODO: write test suite for functions that give an output
    tests_string(&total, &success, &failure);
    printf("\nTEST SUITE DONE:\n  TOTAL   : %d\n  SUCCESS : %d\n  FAILURE : %d\n", total, success, failure);
}

void    exec_command() {
    char words[4][256];
    size_t i = 0;
    size_t j;

    line[index] = 0;
    index = 0;

    for (size_t k = 0; k < 4; k++) {
        while (line[i] == ' ') i++;
        for (j = 0; line[i] >= 0x21 && line[i] <= 0x7E; j++) words[k][j] = line[i++];
        words[k][j] = 0;
    }
    if (!strcmp(words[0], "SET")) {
        if (!strcmp(words[1], "KEYBOARD")) set_keyboard(words[2]);
        else if (!strcmp(words[1], "COLOR")) set_color();//set_color(words[2], words[3]);
    } else if (!strcmp(words[0], "CLEAR")) {
        term_clear();
    } else if (!strcmp(words[0], "TESTS")){
        exec_tests();
    } else if (!strcmp(words[0], "HELP")) {
        printf("WELCOME TO THE KERNEL\n\
THE CLI IS STILL UNDER DEVELOPMENT.\n\
\n\
COMMANDS AVAILABLE:\n\
  SET [OPTION] : IT SETS THE OPTION YOU CHOSE TO THE VALUE YOU DECIDED\n\
      COLOR [FOREGROUND] [BACKGROUND] : 16 COLORS AVAILABLE, 4 BITS VGA PALLET\n\
      KEYBOARD [LAYOUT] : TWO LAYOUT ARE AVAILABLE, FR AND US\n\
  CLEAR        : IT CLEARS THE CLI\n\
  TESTS        : EXECUTE A TEST SUITE FOR THE KERNEL\n\
  HELP         : PRINTS THIS MESSAGE\n");
    }
}
