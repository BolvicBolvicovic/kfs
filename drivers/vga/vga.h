#ifndef VGA_H
#define VGA_H
#include <stddef.h>
#include <stdint.h>
#include "../utils.h"
#include "../../lib/string/string.h"
enum vga_color {
	BLACK = 0,
	BLUE = 1,
	GREEN = 2,
	CYAN = 3,
	RED = 4,
	MAGENTA = 5,
	BROWN = 6,
	LIGHT_GRAY = 7,
	DARK_GREY = 8,
	LIGHT_BLUE = 9,
	LIGHT_GREEN = 10,
	LIGHT_CYAN = 11,
	LIGHT_RED = 12,
	PINK = 13,
	YELLOW = 14,
	WHITE = 15
};

#define VGA_CTRL_REGISTER	0x3D4
#define VGA_DATA_REGISTER	0x3D5
#define VGA_OFFSET_LOW		0x0F
#define VGA_OFFSET_HIGH		0x0E
#define VGA_MEMORY			0xB8000
#define VGA_COLS			0x50
#define VGA_ROWS			0x19

void	term_clear();
void	term_putchar(char c);
void	term_print(const char* str);
#endif
