#ifndef PROCESSES_H
# define PROCESSES_H

#include <stdint.h>
#include "../lib/stdio/stdio.h"
#include "../drivers/descriptor/descriptor.h"
#include "../memory/vmm/vmm.h"

typedef uint32_t	pid_t;
typedef uint32_t	uid_t;
typedef void		(*sighandler_t)(int);

typedef enum
{
	ZOMBIE,
	READY,
	RUNNING,
	THREADED
} process_status;

#define SIGHUP		1			// Hangup detected on controlling terminal or death of controlling process
#define SIGINT		(1 << 2)	// Interrupt from keyboard
#define SIGQUIT		(1 << 3)	// Quit from keyboard
#define SIGILL		(1 << 4)	// Illegal Instruction
#define SIGTRAP		(1 << 5)	// Trace/breakpoint trap
#define SIGABRT		(1 << 6)	// Abort signal from abort(3)
#define SIGIOT		SIGABRT		// IOT trap. A synonym for SIGABRT
#define SIGBUS		(1 << 7)	// Bus error (bad memory access)
#define SIGFPE		(1 << 8)	// Erroneous arithmetic operation
#define SIGKILL		(1 << 9)	// Kill signal
#define SIGUSR1		(1 << 10)	// User-defined signal 1
#define SIGSEGV		(1 << 11)	// Invalid memory reference
#define SIGUSR2		(1 << 12)	// User-defined signal 2
#define SIGPIPE		(1 << 13)	// Broken pipe: write to pipe with no readers; see pipe(7)
#define SIGALRM		(1 << 14)	// Timer signal from alarm(2)
#define SIGTERM		(1 << 15)	// Termination signal
#define SIGSTKFLT	(1 << 16)	// Stack fault on coprocessor (unused)
#define SIGCHLD		(1 << 17) 	// Child stopped, terminated, or continued
#define SIGCONT  	(1 << 18)	// Continue if stopped
#define SIGSTOP		(1 << 19)	// Stop process
#define SIGTSTP		(1 << 20)	// Stop typed at terminal
#define SIGTTIN		(1 << 21)	// Terminal input for background process
#define SIGTTOU		(1 << 22)	// Terminal output for background process
#define SIGURG		(1 << 23)	// Urgent condition on socket (4.2BSD)
#define SIGXCPU		(1 << 24)	// CPU time limit exceeded (4.2BSD); see setrlimit(2)
#define SIGXFSZ		(1 << 25)	// File size limit exceeded (4.2BSD); see setrlimit(2)
#define SIGVTALRM	(1 << 26)	// Virtual alarm clock (4.2BSD)
#define SIGPROF		(1 << 27)	// Profiling timer expired
#define SIGWINCH	(1 << 28)	// Window resize signal (4.3BSD, Sun)
#define SIGIO		(1 << 29)	// I/O now possible (4.2BSD)
#define SIGPOLL		SIGIO		// Pollable event (Sys V); synonym for SIGIO
#define SIGPWR		(1 << 30)	// Power failure (System V)
#define SIGSYS		(1 << 31)	// Bad system call (SVr4); see also seccomp(2)
#define SIGUNUSED	SIGSYS	// Synonymous with SIGSYS

// TODO: make frame size dependant on if process is user or not
#define FRAME_SIZE 36

typedef enum
{
	KPROC,
	UPROC
} proc_type;

typedef struct
{
	proc_type	type;
	uint32_t*	code;
	uint32_t	code_size;
	uint32_t*	data;
	uint32_t	data_size;
	uint32_t	entry;
} proc_info_t;

typedef struct
{
	uint32_t	_link;
	uint32_t	esp0;
	uint32_t	ss0;
	uint32_t	esp1;
	uint32_t	ss1;
	uint32_t	esp2;
	uint32_t	ss2;
	uint32_t	cr3;
	uint32_t	eip;
	uint32_t	eflags;
	uint32_t	eax, ecx, edx, ebx;
	uint32_t	esp, ebp, esi, edi;
	uint32_t	es, cs, ss;
	uint32_t	ds, fs, gs;
	uint32_t	ldtr;
	uint16_t	trap;
	uint16_t	io_permission_bitmap;
	uint32_t	ssp;
} __attribute__((__packed__)) tss_t;

typedef struct
{
	p_dir*		dir;
	
	uint32_t	code_start;
	uint32_t	code_end;

	uint32_t	data_start;
	uint32_t	data_end;

	uint32_t	stack;
	uint32_t	stack_base;
	
	// uint32_t	heap_start;
	// uint32_t	heap_end;

} mm_t;

typedef struct
{
	// Note: ID and Status
	pid_t			pid;
	process_status	status;
	uint32_t		exit_code;
	// TODO: add exit code and exit signal

	// Note: Scheduling
	// TODO: check scheduling info
	uint32_t		next;

	// Note: Memory managment
	// Note: if mm == 0 then kernel process else user process
	mm_t*			mm;
	uint32_t*		k_stack;
	uint8_t*		k_stack_base;

	// Note: Relationships
	uint32_t		parent;
	//pid_t			children[16];
	//pid_t			fds[32];

	// TODO: when fs exists, add it here

	// Note: Signals
	// TODO: add signal handlers
	uint32_t		pending_signals;

	// Note: Credentials & Security
	// TODO: add credentials and group ids
	uid_t			uid;

	//TODO: add ressource limit, CPU time and context switch count
} process;

void			init_multitasking(void);

int				queue_signal(pid_t, uint32_t s);
int				update_status(pid_t, process_status);
// TODO: look up best way to implement sockets between processes
// TODO: Function to work on the memory of the process (I guess with heap and stack)??
pid_t			create_process(proc_info_t*);
pid_t			fork_process(uint32_t* esp);
void			exit_user_process(uint32_t status);

void			schedule(void);



/* SYSCALLS  */

pid_t			fork(void);
pid_t			wait(int* wstatus);
void			exit(int status);
uid_t			getuid(void);
sighandler_t	signal(int signum, sighandler_t);
int				kill(pid_t, int signal);

/* END SYSCALLS  */

#endif
