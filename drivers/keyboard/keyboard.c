#include "keyboard.h"

#define BACKSPACE 0x0E
#define BACKSPACE_CHAR 0x7F
#define ENTER 0x1C
#define SC_MAX 57
#define KEYBOARD_INPUT_BUFFER 0x60

const char sc_ascii[2][58] = {
        {
            '?', '?', '1', '2', '3', '4', '5', '6',
            '7', '8', '9', '0', '-', '=', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y',
            'U', 'I', 'O', 'P', '[', ']', '\n', '?', 'A', 'S', 'D', 'F', 'G',
            'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z', 'X', 'C', 'V',
            'B', 'N', 'M', ',', '.', '/', '?', '?', '?', ' '
        },
        {
            '?', '?', '1', '2', '3', '4', '5', '6',
            '7', '8', '9', '0', '-', '=', '?', '?', 'A', 'Z', 'E', 'R', 'T', 'Y',
            'U', 'I', 'O', 'P', '[', ']', '\n', '?', 'Q', 'S', 'D', 'F', 'G',
            'H', 'J', 'K', 'L', 'M', '%', '*', '?', '\\', 'W', 'X', 'C', 'V',
            'B', 'N', ',', ';', ':', '!', '?', '?', '?', ' '
        }
};

static size_t keyboard_index = 0;
static size_t settings_index = 0;
current_screen_t current_screen = { .type = SHELL, .lists = { {.null = NULL}, {.null = NULL}, {.null = NULL} } };

static void handle_settings(uint8_t scancode) {
    uint8_t list_limit = 0;
    switch (sc_ascii[keyboard_index][scancode]) {
        case 'Q':
            term_set_color(vga_entry_color(current_screen.lists[1].list.current_item_index, current_screen.lists[0].list.current_item_index));
            keyboard_index = current_screen.lists[2].list.current_item_index;
            settings_index = 0;
            term_clear();
            enable_cursor();
            current_screen.type = SHELL;
            break;
        case 'J' :
            clear_selector(current_screen.lists[settings_index].list.list_vga_index);
            if (settings_index == 0) settings_index = 2;
            else settings_index -= 1;
            draw_selector(current_screen.lists[settings_index].list.list_vga_index);
            break;
        case 'K' :
            clear_selector(current_screen.lists[settings_index].list.list_vga_index);
            if (settings_index == 2) settings_index = 0;
            else settings_index += 1;
            draw_selector(current_screen.lists[settings_index].list.list_vga_index);
            break;
        case 'H' :
            if (settings_index == 2) list_limit = 1; else list_limit = 15;
            clear_selector(current_screen.lists[settings_index].list.list_vga_index);
            if (current_screen.lists[settings_index].list.current_item_index == 0)
                current_screen.lists[settings_index].list.current_item_index = list_limit;
            else current_screen.lists[settings_index].list.current_item_index -= 1;
            draw_name(
                current_screen.lists[settings_index].list.list,
                current_screen.lists[settings_index].list.current_item_index,
                current_screen.lists[settings_index].list.list_vga_index
            );
            draw_selector(current_screen.lists[settings_index].list.list_vga_index);
            break;
        case 'L' :
            if (settings_index == 2) list_limit = 1; else list_limit = 15;
            clear_selector(current_screen.lists[settings_index].list.list_vga_index);
            if (current_screen.lists[settings_index].list.current_item_index == list_limit)
                current_screen.lists[settings_index].list.current_item_index = 0;
            else current_screen.lists[settings_index].list.current_item_index += 1;
            draw_name(
                current_screen.lists[settings_index].list.list,
                current_screen.lists[settings_index].list.current_item_index,
                current_screen.lists[settings_index].list.list_vga_index
            );
            draw_selector(current_screen.lists[settings_index].list.list_vga_index);
            break;
    }
}

static void keyboard_callback(registers_t* regs) {
    uint8_t scancode = port_byte_in(KEYBOARD_INPUT_BUFFER);
    asm volatile("cli");
    if (scancode > SC_MAX) return;
    if (current_screen.type == SHELL) {
        if (scancode == BACKSPACE) {
            term_backspace();
            cmd_add_char(BACKSPACE_CHAR);
        } else if (scancode == ENTER) {
            term_print(&sc_ascii[keyboard_index][scancode], 1);
            exec_command();
        } else {
            term_print(&sc_ascii[keyboard_index][scancode], 1);
            cmd_add_char(sc_ascii[keyboard_index][scancode]);
        }
    } else {
        handle_settings(scancode);
    }
}

void init_keyboard() {
    register_interrupt_handler(IRQ1, &keyboard_callback);
}
