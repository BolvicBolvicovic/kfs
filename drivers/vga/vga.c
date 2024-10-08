#include "vga.h"

static		uint8_t		term_color	= 0x0;

uint8_t     term_get_color() { return term_color; }

inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}

void enable_cursor() {
// Magic numbers are masks used to retain the information stored in the 2 and 3 higher bits of the register
    port_byte_out(VGA_CTRL_REGISTER, CURSOR_START_REGISTER);
    port_byte_out(VGA_DATA_REGISTER, (port_byte_in(VGA_DATA_REGISTER) & 0xC0) | CURSOR_START);
    port_byte_out(VGA_CTRL_REGISTER, CURSOR_END_REGISTER);
    port_byte_out(VGA_DATA_REGISTER, (port_byte_in(VGA_DATA_REGISTER) & 0xE0) | CURSOR_END);
}

void disable_cursor() {
    port_byte_out(VGA_CTRL_REGISTER, CURSOR_START_REGISTER);
    port_byte_out(VGA_DATA_REGISTER, CURSOR_DISABLE_BIT);
}

static void	set_cursor(int offset) {
	offset /= 2;
	port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
	port_byte_out(VGA_DATA_REGISTER, (unsigned char)(offset >> 8));
	port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
	port_byte_out(VGA_DATA_REGISTER, (unsigned char)(offset & 0xFF));
}

static int	get_cursor() {
    int offset = 0;
	port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
	offset |= port_byte_in(VGA_DATA_REGISTER) << 8;
	port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
	offset |= port_byte_in(VGA_DATA_REGISTER);
	return offset * 2;
}

void	term_set_color(uint8_t color) {
	term_color = color;
}

static void	term_put_entry_at(unsigned char c, uint8_t color, size_t offset) {
	unsigned char* vga_memory = (unsigned char*)VGA_MEMORY;
	vga_memory[offset] = c;
	vga_memory[offset + 1] = color;
}

static uint8_t term_get_entry_at(size_t offset) {
	unsigned char* vga_memory = (unsigned char*)VGA_MEMORY;
    return vga_memory[offset];
}

static inline int	get_offset(int col, int row) {
	return 2 * (row * VGA_COLS + col);
}

void	term_clear() {
	for (int index = 0; index < VGA_COLS * VGA_ROWS; index++) {
		term_put_entry_at(' ', term_color, index * 2);
	}
	set_cursor(get_offset(0, 0));
}

static int		term_scroll(int offset) {
	memcpy(
		(char*)(get_offset(0, 0) + VGA_MEMORY),
		(char*)(get_offset(0, 1) + VGA_MEMORY),
		2 * VGA_COLS * (VGA_ROWS - 1)
	);
	for (int col = 0; col < VGA_COLS; col++) {
		term_put_entry_at(' ', term_color, get_offset(col, VGA_ROWS - 1));
	}
	return offset - 2 * VGA_COLS;
}

static inline int	get_row(int offset) {
	return offset / (2 * VGA_COLS);
}

static inline int	move_offset_to_newline(int offset) {
	return get_offset(0, get_row(offset) + 1);
}

inline int   start_of_line(int offset) {
    if (offset % (2 * VGA_COLS) == 0) return 1;
    return 0;
}

void    term_backspace() {
    int offset = get_cursor() - 2;
    if (offset >= 0 && !start_of_line(offset + 2)) {
        term_put_entry_at(' ', term_color, offset);
        set_cursor(offset);
    }
}

inline void	term_print(const char* str, size_t n) {
	int	offset = get_cursor();
	for (size_t i = 0; str[i] && i < n; i++) {
		if (offset >= VGA_ROWS * VGA_COLS * 2) offset = term_scroll(offset);
		switch (str[i]) {
			case '\n':
				offset = move_offset_to_newline(offset);
				break;
			default:
				term_put_entry_at(str[i], term_color, offset);
				offset += 2;
		}
	}
	set_cursor(offset);
}
