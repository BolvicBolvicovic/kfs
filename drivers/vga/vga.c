#include "vga.h"

static u8	term_color	= 0;
static u32	term_base	= 0;
static u32	term_end	= VGA_ROWS;

inline u8	term_get_color() { return term_color; }

inline uint8_t
vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}

void
enable_cursor(void)
{
	// Magic numbers are masks used to retain the information stored in the 2 and 3 higher bits of the register
	port_byte_out(VGA_CTRL_REGISTER, CURSOR_START_REGISTER);
	port_byte_out(VGA_DATA_REGISTER, (port_byte_in(VGA_DATA_REGISTER) & 0xC0) | CURSOR_START);
	port_byte_out(VGA_CTRL_REGISTER, CURSOR_END_REGISTER);
	port_byte_out(VGA_DATA_REGISTER, (port_byte_in(VGA_DATA_REGISTER) & 0xE0) | CURSOR_END);
}

void
disable_cursor(void)
{
	port_byte_out(VGA_CTRL_REGISTER, CURSOR_START_REGISTER);
	port_byte_out(VGA_DATA_REGISTER, CURSOR_DISABLE_BIT);
}

static void
set_cursor(s32 offset)
{
	offset /= 2;
	port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
	port_byte_out(VGA_DATA_REGISTER, (u8)(offset >> 8));
	port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
	port_byte_out(VGA_DATA_REGISTER, (u8)(offset & 0xFF));
}

static int
get_cursor(void)
{
	s32	offset = 0;

	port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
	offset |= port_byte_in(VGA_DATA_REGISTER) << 8;
	port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
	offset |= port_byte_in(VGA_DATA_REGISTER);
	return offset * 2;
}

inline void
term_set_color(u8 color)
{
	term_color = color;
}

static inline void
term_put_entry_at(u8 c, u8 color, u32 offset)
{
	u8* vga_memory = (u8*)VGA_MEMORY;
	vga_memory[offset] = c;
	vga_memory[offset + 1] = color;
}

static inline u8
term_get_entry_at(u32 offset)
{
	u8*	vga_memory = (u8*)VGA_MEMORY;

	return vga_memory[offset];
}

static inline s32
get_offset(s32 col, s32 row)
{
	return 2 * (row * VGA_COLS + col);
}

void
term_clear(void)
{
	for (s32 i = VGA_COLS * term_base; i < VGA_COLS * term_end; i++)
		term_put_entry_at(' ', term_color, i * 2);

	set_cursor(get_offset(0, term_base));
}

static inline s32
term_scroll(s32 offset)
{
	memcpy(
		(char*)(get_offset(0, term_base) + VGA_MEMORY),
		(char*)(get_offset(0, term_base + 1) + VGA_MEMORY),
		2 * VGA_COLS * (VGA_ROWS - term_base - 1)
	);

	for (s32 col = 0; col < VGA_COLS; col++)
		term_put_entry_at(' ', term_color, get_offset(col, VGA_ROWS - term_base - 1));

	return offset - 2 * VGA_COLS;
}

static inline s32
get_row(s32 offset)
{
	return offset / (2 * VGA_COLS);
}

static inline s32
move_offset_to_newline(s32 offset)
{
	return get_offset(0, get_row(offset) + 1);
}

inline s32
start_of_line(s32 offset)
{
    if (offset % (2 * VGA_COLS) == 0)
	    return 1;

    return 0;
}

void
term_backspace(void)
{
	s32	offset = get_cursor() - 2;
	
	if (offset >= 0 && !start_of_line(offset + 2))
	{
		term_put_entry_at(' ', term_color, offset);
		set_cursor(offset);
	}
}

inline void
term_print(const char* str, u32 n)
{
	s32	offset = get_cursor();

	for (u32 i = 0; str[i] && i < n; i++)
	{
		switch (str[i])
		{
		case '\n':
			offset = move_offset_to_newline(offset);
			break;
		default:
			term_put_entry_at(str[i], term_color, offset);
			offset += 2;
		}
		
		if (offset >= VGA_ROWS * VGA_COLS * 2)
			offset = term_scroll(offset);

	}

	set_cursor(offset);
}

inline void
term_set_offsets(u32 base, u32 end)
{
	term_base	= base;
	term_end	= end;

	set_cursor(get_offset(0, base));
}
