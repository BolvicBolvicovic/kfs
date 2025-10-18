#include "syscall.h"

static void
sys_read(registers_t* r)
{
	printf("Syscall read : eax == %d\n", r->eax);
}

static void
sys_write(registers_t* r)
{
	if (r->ebx == 0 || r->ebx == 1)
	{
		term_print((const char*)r->ecx, r->edx);
		r->eax = r->edx;
	}
	else
	{
		printf("Writing to anything else than terminal is currently unsupported\n");
	}
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
	extern uint32_t	fork_process(uint32_t*);
	fork_process((uint32_t*)r);
}

// Note: this should not be called from the kernel
static void
sys_exit(registers_t* r)
{
	extern void		exit_user_process(uint32_t, uint32_t*);
	exit_user_process(r->ebx, (uint32_t*)r);
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

static void
sys_mmap(registers_t* r)
{
	extern uint32_t	mmap_user(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

	uint32_t	addr  = r->ebx;
	uint32_t	len   = r->ecx;
	uint32_t	prot  = r->edx;
	uint32_t	flags = r->esi;
	uint32_t	fd    = r->edi;
	uint32_t	off   = r->ebp;

	r->eax = mmap_user(addr, len, prot, flags, fd, off);
}

static void
sys_munmap(registers_t* r)
{
	extern uint32_t	munmap_user(uint32_t, uint32_t);
	uint32_t	addr  = r->ebx;
	uint32_t	len   = r->ecx;

	r->eax = munmap_user(addr, len);
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
	syscall_tab[9]		= &sys_mmap;
	syscall_tab[11]		= &sys_munmap;
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
