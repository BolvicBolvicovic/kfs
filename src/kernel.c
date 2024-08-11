#include <stddef.h>
#include <stdint.h>
/*
#if defined(__linux__)
	#error "This code must be compiled with a cross-compiler"
#elif !defined(__i386__)
	#error "This code must be compiled with an x86-elf compiler"
#endif
*/
volatile	uint16_t*	vga_buffer	= (uint16_t*)0xB8000;
const		int			VGA_COLS	= 80;
const		int			VGA_ROWS	= 25;
			int			term_col	= 0;
			int			term_row	= 0;
			uint8_t		term_color	= 0x0F;

void	term_init() {
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

void	kernel_main() {
	term_init();

	term_print("Hello, World!\n");
	term_print("Welcome to the kernel.\n");
}
