#include "keyboard.h"

static char line[256];
static size_t index = 0;
inline void cmd_add_char(uint8_t c) {
    if (c == 0x7F && index - 1 >= 0) index--;
    else if (index < 256) line[index++] = c;
}

void    exec_command() {
    line[index] = 0;
    index = 0;
    char words[4][256];
    size_t i = 0;
    while (line[i]) {
        while (line[i] == ' ') i++;
        size_t j = 0;
        while (line[i] >= 'A' && line[i] <= 'Z') words[0][j++] = line[i++];
        words[0][j] = 0;
        j = 0;
        while (line[i] >= 'A' && line[i] <= 'Z') words[1][j++] = line[i++];
        words[1][j] = 0;
        j = 0;
        while (line[i] >= 'A' && line[i] <= 'Z') words[2][j++] = line[i++];
        words[2][j] = 0;
        j = 0;
        while (line[i] >= 'A' && line[i] <= 'Z') words[3][j++] = line[i++];
        words[3][j] = 0;
    }
    if (strcmp(words[0], "SET\0")) return;
    if (!strcmp(words[1], "KEYBOARD\0")) set_keyboard(words[2]);
    else if (!strcmp(words[2], "TERM_COLOR\0")) set_color(words[2], words[3]);
}
