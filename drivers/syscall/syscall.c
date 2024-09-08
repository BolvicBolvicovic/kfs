#include "syscall.h"

void sys_read(registers_t* r) {
	printf("Syscall read : eax == %d\n", r->eax);
}

void sys_write(registers_t* r) {
	printf("Syscall write: eax == %d\n", r->eax);
}

void sys_open(registers_t* r) {
	printf("Syscall open : eax == %d\n", r->eax);
}

void sys_close(registers_t* r) {
	printf("Syscall close: eax == %d\n", r->eax);
}

void sys_stat(registers_t* r) {
	printf("Syscall stat : eax == %d\n", r->eax);
}

#define NB_OF_SC 5

isr_t syscall_tab[NB_OF_SC] = {
	sys_read,
	sys_write,
	sys_open,
	sys_close,
	sys_stat
	// Write all handlers here based on Linux system call table
};

void syscall_callback(registers_t* r) {
	if (r->eax < NB_OF_SC) syscall_tab[r->eax](r);
}

void init_syscall() {
	register_interrupt_handler(SYSCALL, &syscall_callback);
}
