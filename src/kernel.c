#include "kernel.h"
#if defined(__linux__)
	#error "This code must be compiled with a cross-compiler"
#elif !defined(__i386__)
	#error "This code must be compiled with an x86-elf compiler"
#endif

void	kernel_main() {
	term_init();

	term_print("Hello, World!\n");
	term_print("Welcome to the kernel.\n");
    term_print("You're great!\n");
    char   bath[10];
    strcpy(bath, "bath\n");
    printf("printf result: %d\n", printf("I have a %s\n", bath));
}
