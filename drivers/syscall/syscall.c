#include "syscall.h"

static void
sys_read(registers_t* r)
{
	printf("Syscall read : eax == %d\n", r->eax);
}

static void
sys_write(registers_t* r)
{
	printf("Syscall write: eax == %d\n", r->eax);
}

static void
sys_open(registers_t* r)
{
	printf("Syscall open : eax == %d\n", r->eax);
}

static void
sys_close(registers_t* r)
{
	printf("Syscall close: eax == %d\n", r->eax);
}

static void
sys_stat(registers_t* r)
{
	printf("Syscall stat : eax == %d\n", r->eax);
}

static void
sys_signal(registers_t* r)
{
	printf("Syscall signal : eax == %d\n", r->eax);
}

static void
sys_fork(registers_t* r)
{
	extern uint32_t	fork_process(uint32_t* esp);
	fork_process((uint32_t*)r);
}

static void
sys_exit(registers_t* r)
{
	printf("Syscall exit : eax == %d\n", r->eax);
}

static void
sys_kill(registers_t* r)
{
	printf("Syscall kill : eax == %d\n", r->eax);
}

static void
sys_getuid(registers_t* r)
{
	printf("Syscall getuid : eax == %d\n", r->eax);
}

static void
sys_wait(registers_t* r)
{
	printf("Syscall wait : eax == %d\n", r->eax);
}

#define NB_OF_SC 468

// TODO: Write all handlers here based on Linux system call table when all implemented
isr_t syscall_tab[NB_OF_SC];

static void
syscall_callback(registers_t* r)
{
	if (r->eax < NB_OF_SC) syscall_tab[r->eax](r);
}

void
init_syscall(void)
{
	syscall_tab[0]		= &sys_read;
	syscall_tab[1]		= &sys_write;
	syscall_tab[2]		= &sys_open;
	syscall_tab[3]		= &sys_close;
	syscall_tab[4]		= &sys_stat;
	// TODO: change to rt_sigaction
	syscall_tab[13]		= &sys_signal;
	syscall_tab[57]		= &sys_fork;
	syscall_tab[60]		= &sys_exit;
	syscall_tab[62]		= &sys_kill;
	syscall_tab[102]	= &sys_getuid;
	// TODO: change with sys_waitid
	syscall_tab[247]	= &sys_wait;
	register_interrupt_handler(SYSCALL, &syscall_callback);
}
