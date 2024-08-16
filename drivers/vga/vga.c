#include "vga.h"

static		uint8_t		term_color	= 0x0;

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

static void	set_cursor(int offset) {
	offset /= 2;
	port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
	port_byte_out(VGA_DATA_REGISTER, (unsigned char)(offset >> 8));
	port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
	port_byte_out(VGA_DATA_REGISTER, (unsigned char)(offset & 0xFF));
}

static int	get_cursor() {
	port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
	int	offset = port_byte_in(VGA_DATA_REGISTER) << 8;
	port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
	offset += port_byte_in(VGA_DATA_REGISTER);
	return offset * 2;
}

void	term_set_color(uint8_t color) {
	term_color = color;
}

void	term_clear() {
	unsigned char* vga_memory = (unsigned char*)VGA_MEMORY;
    term_color = vga_entry_color(RED, CYAN);
	for (int index = 0; index < VGA_COLS * VGA_ROWS; index++) {
		vga_memory[index] = ((uint16_t)term_color << 0x8) | ' ';
	}
}

static void		term_put_entry_at(unsigned char c, uint8_t color, size_t offset) {
	unsigned char* vga_memory = (unsigned char*)VGA_MEMORY;
	vga_memory[offset] = vga_entry(c, color);
	vga_memory[offset + 1] = vga_entry(' ', color);
}

static inline int	get_offset(int col, int row) {
	return 2 * row * (VGA_COLS + col);
}

static int		term_scroll(int offset) {
	memcpy(
		(void*)(get_offset(0, 1) + VGA_MEMORY),
		(void*)(get_offset(0, 0) + VGA_MEMORY),
		2 * VGA_COLS * (VGA_ROWS - 1)
	);
	for (int col = 0; col < VGA_COLS; col++) {
		term_put_entry_at(' ', term_color, get_offset(col, VGA_ROWS - 1));
	}
	return offset - 2 * VGA_COLS;
}


static inline int	move_offset_to_newline(int offset) {
	return 2 * VGA_COLS + offset;
}

void	term_putchar(char c) {
	int	offset = get_cursor();
	unsigned char	uc = c;
	
	if (offset >= VGA_ROWS * VGA_COLS * 2) offset = term_scroll(offset);
	switch (uc) {
		case '\n':
			offset = move_offset_to_newline(offset);
			break;
		default:
			term_put_entry_at(uc, term_color, offset);
	}
	set_cursor(offset);
}

inline void	term_print(const char* str) {
	for (size_t i = 0; str[i]; i++) term_putchar(str[i]);
}
