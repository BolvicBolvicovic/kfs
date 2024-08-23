#include "kernel.h"
#if defined(__linux__)
	#error "This code must be compiled with a cross-compiler"
#elif !defined(__i386__)
	#error "This code must be compiled with an x86-elf compiler"
#endif

void	kernel_main() {
	term_set_color(vga_entry_color(YELLOW, GREEN));
	term_clear();
	isr_install();
	init_keyboard();
	init_timer(50);
//	initialise_paging();
	asm volatile("sti\n\t");
}
