#include "vga.h"
volatile	uint16_t*	vga_buffer	= (uint16_t*)0xB8000;
const		int			VGA_COLS	= 80;
const		int			VGA_ROWS	= 25;
			int			term_col	= 0;
			int			term_row	= 0;
			uint8_t		term_color	= 0;

void    set_term_color(uint8_t foreground, uint8_t background) {
   term_color = background << 4 | foreground; 
}

void	term_init() {
    set_term_color(RED, GREEN);
	for (int col = 0; col < VGA_COLS; col++) {
		for (int row = 0; row < VGA_ROWS; row++) {
			const size_t	index = (VGA_COLS * row) + col;
			vga_buffer[index] = ((uint16_t)term_color << 8) | ' ';
		}
	}
}

void	term_putchar(char c) {
	switch (c) {
		case '\n': {
			term_col = 0;
			term_row += 1;
			break;
		}
		default: {
			const size_t	index = (VGA_COLS * term_row) + term_col;
			vga_buffer[index] = ((uint16_t)term_color << 8) | c;
			term_col += 1;
			break;
		}
	}
	if (term_col >= VGA_COLS) {
		term_col = 0;
		term_row += 1;
	}
	if (term_row >= VGA_ROWS) {
		term_col = 0;
		term_row = 0;
	}
}

void	term_print(const char* str) {
	for (size_t i = 0; str[i]; i++) {
		term_putchar(str[i]);
	}
}
