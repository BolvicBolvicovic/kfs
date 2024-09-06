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
    tests_stdlib(&total, &success, &failure);
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

void int_0x0() { asm volatile("int $0x0"); }
void int_0x1() { asm volatile("int $0x1"); }
void int_0x2() { asm volatile("int $0x2"); }
void int_0x3() { asm volatile("int $0x3"); }
void int_0x4() { asm volatile("int $0x4"); }
void int_0x5() { asm volatile("int $0x5"); }
void int_0x6() { asm volatile("int $0x6"); }
void int_0x7() { asm volatile("int $0x7"); }
void int_0x8() { asm volatile("int $0x8"); }
void int_0x9() { asm volatile("int $0x9"); }
void int_0xA() { asm volatile("int $0xA"); }
void int_0xB() { asm volatile("int $0xB"); }
void int_0xC() { asm volatile("int $0xC"); }
void int_0xD() { asm volatile("int $0xD"); }
void int_0xE() { asm volatile("int $0xE"); }
void int_0xF() { asm volatile("int $0xF"); }
void int_0x10() { asm volatile("int $0x10"); }
void int_0x11() { asm volatile("int $0x11"); }
void int_0x12() { asm volatile("int $0x12"); }
void int_0x13() { asm volatile("int $0x13"); }
void int_0x14() { asm volatile("int $0x14"); }
void int_0x15() { asm volatile("int $0x15"); }
void int_0x16() { asm volatile("int $0x16"); }
void int_0x17() { asm volatile("int $0x17"); }
void int_0x18() { asm volatile("int $0x18"); }
void int_0x19() { asm volatile("int $0x19"); }
void int_0x1A() { asm volatile("int $0x1A"); }
void int_0x1B() { asm volatile("int $0x1B"); }
void int_0x1C() { asm volatile("int $0x1C"); }
void int_0x1D() { asm volatile("int $0x1D"); }
void int_0x1E() { asm volatile("int $0x1E"); }
void int_0x1F() { asm volatile("int $0x1F"); }
void int_0x20() { asm volatile("int $0x20"); }
void int_0x21() { asm volatile("int $0x21"); }
void int_0x22() { asm volatile("int $0x22"); }
void int_0x23() { asm volatile("int $0x23"); }
void int_0x24() { asm volatile("int $0x24"); }
void int_0x25() { asm volatile("int $0x25"); }

typedef void (*inter_func)(void);

inter_func tab[] = {
    int_0x0, int_0x1, int_0x2, int_0x3, 
    int_0x4, int_0x5, int_0x6, int_0x7,
    int_0x8, int_0x9, int_0xA, int_0xB,
    int_0xC, int_0xD, int_0xE, int_0xF,
    int_0x10, int_0x11, int_0x12, int_0x13,
    int_0x14, int_0x15, int_0x16, int_0x17,
    int_0x18, int_0x19, int_0x1A, int_0x1B,
    int_0x1C, int_0x1D, int_0x1E, int_0x1F,
    int_0x20, int_0x21, int_0x22, int_0x23,
    int_0x24, int_0x25
};

void do_interrupt(char* nb) {
    int n = atoi(nb);
    if (n < 0 || n > 0x25) return;
    tab[n]();
}

void shut_down() {
    asm volatile(
        "movw $0x2000, %ax\n"
        "movw $0x604, %dx\n"
        "outw %ax, %dx\n"
        "hlt"
    );
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
    } else if (!strcmp(words[0], "EXIT")){
        shut_down();
    } else if (!strcmp(words[0], "HALT")){
        asm volatile("hlt\n\t");
    } else if (!strcmp(words[0], "STACK")){
        print_stack();
    } else if (!strcmp(words[0], "INT")){
        do_interrupt(words[1]);
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
  EXIT         : SHUT DOWN THE KERNEL AND THE VM\n\
  HALT         : HALTS THE KERNEL\n\
  STACK        : PRINTS STACK\n\
  INT   [NUM]  : DOES INTERRUPTION [NUM]\n\
  TEST         : EXECUTES A TEST SUITE FOR THE KERNEL\n\
  HELP         : PRINTS THIS MESSAGE\n");
    }
}
