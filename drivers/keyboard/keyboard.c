#include "keyboard.h"

#define BACKSPACE 0x0E
#define ENTER 0x1C
#define SC_MAX 57

/*const char *sc_name[] = {"ERROR", "Esc", "1", "2", "3", "4", "5", "6",
                         "7", "8", "9", "0", "-", "=", "Backspace", "Tab", "Q", "W", "E",
                         "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "Lctrl",
                         "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`",
                         "LShift", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".",
                         "/", "RShift", "Keypad *", "LAlt", "Spacebar"};
*/
const char sc_ascii[] = {'?', '?', '1', '2', '3', '4', '5', '6',
                         '7', '8', '9', '0', '-', '=', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y',
                         'U', 'I', 'O', 'P', '[', ']', '\n', '?', 'A', 'S', 'D', 'F', 'G',
                         'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z', 'X', 'C', 'V',
                         'B', 'N', 'M', ',', '.', '/', '?', '?', '?', ' '};

static void keyboard_callback(registers_t* regs) {
    uint8_t scancode = port_byte_in(0x60);
    asm volatile("cli");
    if (scancode > SC_MAX) return;
    if (scancode == BACKSPACE) {
        term_backspace();
        cmd_add_char(sc_ascii[scancode]);
    } else if (scancode == ENTER) {
        term_print(&sc_ascii[scancode], 1);
        exec_command();
    } else {
        term_print(&sc_ascii[scancode], 1);
        cmd_add_char(sc_ascii[scancode]);
    }
}

void init_keyboard() {
    register_interrupt_handler(IRQ1, &keyboard_callback);
}
