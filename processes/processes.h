#ifndef PROCESSES_H
# define PROCESSES_H

#include <stdint.h>
#include "../lib/stdio/stdio.h"

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

typedef struct
{
	// Note: Right now, it is an index so no need to store it
	// pid_t			pid;
	uid_t			uid;
	process_status	status;
	uint32_t		signals;
	//pid_t			parent;
	// TODO: check scheduling info
	// TODO: dynamize children, fd table, heap and stack
	//pid_t			children[16];
	//pid_t			fds[32];
	uint32_t*		stack;
	uint8_t*		stack_base;
	uint8_t*		heap;
	uint32_t		next;
} process;

int				queue_signal(pid_t p, uint32_t s);
int				update_status(pid_t p, process_status s);
// TODO: look up best way to implement sockets between processes
// TODO: Function to work on the memory of the process (I guess with heap and stack)??
pid_t			create_process(void (*entry)(void));
pid_t			fork_process(pid_t p);
void			schedule(void);
void			init_multitasking(void);


/* SYSCALLS  */

pid_t			wait(int* wstatus);
void			exit(int status);
uid_t			getuid(void);
sighandler_t	signal(int signum, sighandler_t handler);
int				kill(pid_t pid, int signal);

/* END SYSCALLS  */

// Testing function: Note that the example under is not semantically right.
//void 			exec_process(uint32_t* addr, uint32_t* function, uint32_t size);

#endif
