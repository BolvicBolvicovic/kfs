#ifndef PROCESSES_H
# define PROCESSES_H

#include <c_types.h>
#include <compiler.h>
#include <linked_list.h>
#include <stdio.h>
#include <processes/locks/spinlock.h>
#include <drivers/descriptor/descriptor.h>
#include <memory/vmm/vmm.h>

typedef u32	pid_t;
typedef u32	uid_t;
typedef void	(*sighandler_t)(int);

typedef enum
{
	ZOMBIE,
	READY,
	RUNNING,
	SLEEPING,
	THREADED
} process_status;

#define SIGHUP		1		// Hangup detected on controlling terminal or death of controlling process
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
#define SIGUNUSED	SIGSYS		// Synonymous with SIGSYS

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
	u32*		code;
	u32		code_size;
	u32*		data;
	u32		data_size;
	u32		entry;
} proc_info_t;

typedef struct
{
	u32	_link;
	u32	esp0;
	u32	ss0;
	u32	esp1;
	u32	ss1;
	u32	esp2;
	u32	ss2;
	u32	cr3;
	u32	eip;
	u32	eflags;
	u32	eax, ecx, edx, ebx;
	u32	esp, ebp, esi, edi;
	u32	es, cs, ss;
	u32	ds, fs, gs;
	u32	ldtr;
	u16	trap;
	u16	io_permission_bitmap;
	u32	ssp;
} __packed tss_t;

typedef struct mm_s
{
	u32		dir;
	
	u32		code_start;
	u32		code_end;

	u32		data_start;
	u32		data_end;

	u32		stack;
	u32		stack_base;
	
	// u32	heap_start;
	// u32	heap_end;
	spinlock_t	heap_lock;
} mm_t;

typedef struct process_t process_t;
struct process_t
{
	// Note: ID and Status
	pid_t			pid;
	process_status		status;
	u32			exit_code;

	// Note: Scheduling
	// TODO: add && check scheduling info
	process_t*		next;		// Next process address

	// Note: Memory managment
	// Note: if mm == 0 then kernel process else user process
	mm_t*			mm;
	u32*			k_stack;
	u8*			k_stack_base;

	// Note: Relationships
	process_t*		parent;		// Parent process address
	// Note: Store self here so that we don't need to allocate memory for it elsewhere.
	single_ll_node_t	self;
	single_ll_node_t*	children;
	spinlock_t		children_lock;
	// Note: sibilings are parent's children meaning the process contains itself in the sibilings.
	single_ll_node_t**	sibilings;
	//pid_t		fds[32];

	// TODO: when fs exists, add it here

	// Note: Signals
	// TODO: add signal handlers
	u32			pending_signals;
	u32			exit_signal;

	// Note: Credentials & Security
	// TODO: add credentials and group ids
	uid_t			uid;

	//TODO: add ressource limit, CPU time and context switch count
};

void		init_multitasking(void);

s32		queue_signal(pid_t, u32 s);
s32		update_status(pid_t, process_status);
// TODO: look up best way to implement sockets between processes
// TODO: Function to work on the memory of the process (I guess with heap and stack)??
pid_t		create_process(proc_info_t*);
#define KPROC_CREATE(func) create_process(&(proc_info_t){	\
		.type		= KPROC,			\
		.code		= 0,				\
		.code_size	= 0,				\
		.data		= 0,				\
		.data_size	= 0,				\
		.entry		= (func)})
	proc_type	type;
	u32*		code;
	u32		code_size;
	u32*		data;
	u32		data_size;
	u32		entry;
pid_t		fork_process(u32* esp);
void		exit_user_process(u32 status, u32* esp);

/* Name: awaken_processes_push
 * Descrition: pushes a process in the awaken_processe linked list
 * which will be then added to the round-robin list of running processes by the scheduler.
 * */
void		awaken_processes_push(process_t*);

/* Name: running_process_pop 
 * Descrition: turns the running_process into a SLEEPING process and returns it.
 * SLEEPING processes are not pushed back to the READY queue when the scheduler swaps them out.
 * Does not block the scheduler.
 * It is wise to use it with scheduler_lock and scheduler_unlock.
 * */
process_t*	running_process_pop(void);

void		schedule(u32* old_esp);

/* Name: scheduler_lock
 * Descrition: blocks the scheduler from rotating the RUNNING process with the next READY process.
 * */
void		scheduler_lock(void);

/* Name: scheduler_unlock
 * Descrition: unblocks the scheduler, allowing it to rotate the RUNNING process
 * with the next READY process.
 * */
void		scheduler_unlock(void);

/* SYSCALLS  */

pid_t		fork(void);
pid_t		wait(int* wstatus);
void		exit(int status);
uid_t		getuid(void);
sighandler_t	signal(int signum, sighandler_t);
int		kill(pid_t, int signal);

/* END SYSCALLS  */

#endif
