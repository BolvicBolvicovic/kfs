#include "kernel.h"
#if defined(__linux__)
	#error "This code must be compiled with a cross-compiler"
#elif !defined(__i386__)
	#error "This code must be compiled with an x86-elf compiler"
#endif

void	kernel_main() {
	term_clear();

	term_print("Hello, World!\0", 14);
	char k[] = "Welcome to the Kernel\nheeeeeeeelllllllllllll yeah";
	printf("%sHere is a message:\n", k);
}
