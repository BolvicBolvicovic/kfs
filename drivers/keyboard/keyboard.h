#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <stdint.h>
#include <drivers/vga/vga.h>
#include <drivers/descriptor/descriptor.h>

void    init_keyboard();
void    exec_command();
void    cmd_add_char(u8 c);

typedef enum screen_type_t screen_type_t;
enum screen_type_t
{
	SHELL,
	SHELL_DOUBLE,
	SETTINGS,
};

typedef struct list_t list_t;
struct list_t
{
	char** 	list;
	u8	current_item_index;
	u32	list_vga_index;
};

typedef union list_option_t list_option_t;
union list_option_t
{
	list_t	list;
	void*	null;
};


typedef struct current_screen_t current_screen_t;
struct current_screen_t
{
	screen_type_t	type;
	u8		shell_id;
	list_option_t	lists[3];
};


list_option_t   draw_list(char* name, const char** list, u8 list_index, u8 row, u8 col, u8 span);
void            draw_line(char* line, u8 row, u8 col);
void            draw_selector(u32 index);
void            draw_name(char** list, u32 item_index, u32 vga_index);
void            clear_selector(u32 index);

#endif
