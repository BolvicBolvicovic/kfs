#include "keyboard.h"

static char line[256];
static size_t index = 0;
extern current_screen_t current_screen;

const char*  color_list[16] = {
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

const char* keyboard_list[2] = {
    "US",
    "FR"
};

void    init_current_screen(enum vga_color fg, enum vga_color bg) {
	term_set_color(vga_entry_color(fg, bg));
    enable_cursor();
	current_screen.lists[0] = (list_option_t) { .list = {.list = color_list, .current_item_index = bg, .list_vga_index = 0 }};
	current_screen.lists[1] = (list_option_t) { .list = {.list = color_list, .current_item_index = fg, .list_vga_index = 0 }};
	current_screen.lists[2] = (list_option_t) { .list = {.list = keyboard_list, .current_item_index = 0, .list_vga_index = 0 }};
}

static void set() {
    term_clear();
    list_option_t background = draw_list("BACKGROUND", color_list, current_screen.lists[0].list.current_item_index, (VGA_ROWS / 4) * 2, 10, 30);
    list_option_t foreground = draw_list("FORGROUND", color_list, current_screen.lists[1].list.current_item_index, (VGA_ROWS / 2) * 2, 10, 32);
    list_option_t keyboard   = draw_list("KEYBOARD", keyboard_list, current_screen.lists[2].list.current_item_index, ((VGA_ROWS * 3) / 4) * 2, 10, 34);
    if (background.null == NULL || foreground.null == NULL || keyboard.null == NULL) return;
    disable_cursor();
    current_screen.type = SETTINGS;
    current_screen.lists[0] = background;
    current_screen.lists[1] = foreground;
    current_screen.lists[2] = keyboard;
    draw_selector(current_screen.lists[0].list.list_vga_index);
    draw_line("Q Quit / J Up / K Down / H Left / L Right", 2 * (VGA_ROWS - 3), 10);
}

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

void    reboot() {
    asm volatile(
        "cli\n\t"
        "mov $0xFE, %al\n\t"
        "outb %al, $0x64\n\t"
        "hlt"
    );
}

extern uint32_t stack_bottom;
extern uint32_t stack_top;

void print_stack() {
    uint32_t count = 0;
    uint32_t repeat = 0xFFFFFFFF;
    printf(
        "STACK TOP: %p\n"
        "STACK BOT: %p\n",
        &stack_top, &stack_bottom
    );
    for (uint32_t* i = &stack_top; i > &stack_bottom; i--) {
        if (repeat != *i) {
            if (!count) printf("0x%x | ", *i);
            else { printf("0x%x times %d | ", *i, count + 1); count = 0; }
        } else count++;
        repeat = *i;
    }
    printf("\n");
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
        set();
    } else if (!strcmp(words[0], "CLEAR")) {
        term_clear();
    } else if (!strcmp(words[0], "TEST")){
        exec_tests();
    } else if (!strcmp(words[0], "REBOOT")){
        reboot();
    } else if (!strcmp(words[0], "HALT")){
        asm volatile("hlt\n\t");
    } else if (!strcmp(words[0], "STACK")){
        print_stack();
    } else if (!strcmp(words[0], "HELP")) {
        printf("WELCOME TO THE KERNEL\n\
THE CLI IS STILL UNDER DEVELOPMENT.\n\
\n\
COMMANDS AVAILABLE:\n\
  SET          : OPENS THE SETTINGS PAGE\n\
    - COLOR    = 16 COLORS AVAILABLE, 4 BITS VGA PALLET\n\
    - KEYBOARD = TWO LAYOUT ARE AVAILABLE, FR AND US\n\
  CLEAR        : CLEARS THE CLI\n\
  REBOOT       : REBOOTS THE KERNEL\n\
  HALT         : HALTS THE KERNEL\n\
  STACK        : PRINTS STACK\n\
  TEST         : EXECUTES A TEST SUITE FOR THE KERNEL\n\
  HELP         : PRINTS THIS MESSAGE\n");
    }
}
