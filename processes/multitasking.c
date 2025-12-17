#include "processes.h"
#include <atomic.h>
#include <processes/locks/spinlock.h>
#include <memory/allocators/kmalloc.h>
#include <linked_list.h>

#define PT_SIZE		1024
#define STACK_SIZE	0x2000

// Note: From lib/string
extern void*	memcpy(void* d, const void* s, u32 n);
extern void*	memset(void* s, u8 c, u32 n);

// Note: From memory/vmm
extern void	switch_dir(u32 dir);

// Note: From switch.s
extern void	switch_process(u32* new);
extern void	switch_process_user(u32* new_stack, u32 dir);
extern void	start_process(u32* new);
extern void	tss_flush(void);

LINKED_LIST_STRUCT(list_processes_t, process_t);

static list_processes_t ready_processes		= { 0, 0 };
static list_processes_t	awaken_processes	= { 0, 0 };

static tss_t		tss;
// TODO: Use hash table instead with pid as index
static process_t*	process_table[PT_SIZE]	= {0};
static process_t*	kernel_process		= 0;
static u32		pid_count		= 1;


// Note: do not need to use a specific lock since it just tells the scheduler to do its work or not.
static ATOMIC_DEFINE(scheduler_locked);

static SPINLOCK_DEFINE(awaken_processes_lock);
static SPINLOCK_DEFINE(pid_count_lock);

static inline pid_t
new_pid(void)
{
	spinlock_lock(&pid_count_lock);
	u32	start_pid = pid_count;

	do
	{
		if (!process_table[pid_count - 1] || process_table[pid_count - 1]->status == ZOMBIE)
		{
			pid_t	pid	= pid_count;
            		pid_count	= (pid % (PT_SIZE - 1)) + 1;

			spinlock_unlock(&pid_count_lock);

			return pid;
		}
		
		pid_count = (pid_count % (PT_SIZE - 1)) + 1;
	} while (pid_count != start_pid);
	
	spinlock_unlock(&pid_count_lock);

	return 0;
}

static inline process_t*
get_process(pid_t p)
{
	if (p <= 1 || p > PT_SIZE - 1)
	{
		return 0;
	}

	return process_table[p - 1];
}

static void
exit_process(u32 pid)
{
	process_table[pid - 1]->status = ZOMBIE;
	//kfree(ready_processes.head->k_stack_base);
	for (;;)
	{
		asm volatile ("hlt\n\t");
	}
}

void
scheduler_lock(void)
{
	atomic_write(&scheduler_locked, 1);
}

void
scheduler_unlock(void)
{
	atomic_write(&scheduler_locked, 0);
}

void
awaken_processes_push(process_t* p)
{
	p->status = READY;

	spinlock_lock(&awaken_processes_lock);

	if (!awaken_processes.head)
	{
		awaken_processes.head = p;
		awaken_processes.head->next = p;
	}
	else
	{
		awaken_processes.tail->next = p;
	}

	awaken_processes.tail = p;

	spinlock_unlock(&awaken_processes_lock);
}

u32
kgetuid(void)
{
	return ready_processes.head->uid;
}

process_t*
running_process_pop(void)
{
	process_t*	ret = ready_processes.head;

	ret->status	= SLEEPING;
	
	return ret;
}

s32
queue_signal(pid_t p, u32 sig)
{
	if (!p || p > PT_SIZE - 1)
	{
		return 0;
	}

	process_table[p - 1]->pending_signals |= sig;
	return 1;
}

s32
update_status(pid_t p, process_status s)
{
	if (!p || p > PT_SIZE - 1)
	{
		return 0;
	}

	process_table[p - 1]->status = s;
	return 1;
}

pid_t
fork_process(u32* esp)
{
	process_t*	running = ready_processes.head;
	// TODO: Handle user process
	pid_t		fork_pid = new_pid();
	if (!fork_pid) return 0;

	u8*	k_stack_base = (u8*)kmalloc(STACK_SIZE);
	if (!k_stack_base) return 0;

	memset(k_stack_base, 0, STACK_SIZE);

	process_t*	fork = (process_t*)k_stack_base;

	process_table[fork_pid - 1] = fork;

	fork->pid 			= fork_pid;
	fork->uid 			= running->uid;
	fork->parent			= running;
	fork->self.data			= &fork;
	// TODO: check if fork parent and children are the same
	fork->sibilings			= &running->children;
	fork->children			= 0;
	fork->children_lock.counter	= 0;

	spinlock_lock(&running->children_lock);
	single_ll_push(&running->children, &fork->self);
	spinlock_unlock(&running->children_lock);

	fork->status		= READY;
	fork->pending_signals	= 0;
	fork->next		= kernel_process;
	fork->k_stack_base	= k_stack_base;
	u32 used_k_stack	= (u32)(running->k_stack_base + STACK_SIZE) - (u32)esp;
	fork->k_stack		= (u32*)(fork->k_stack_base + STACK_SIZE - used_k_stack);

	memcpy(fork->k_stack, esp, used_k_stack);

	if (running->mm)
	{
		// TODO: check how to handle page directory for parent/child
		fork->mm = memcpy((void*)((u32)(fork->k_stack_base + sizeof(process_t) + 7) & ~3),
				running->mm,
				sizeof(mm_t));
	}
	else
	{
		*(u32*)(fork->k_stack_base + STACK_SIZE - 4) = fork_pid; // Set exit_process input
	}

	*(fork->k_stack + 8)	= 0;
	*(fork->k_stack + 3)	= (u32)fork->k_stack_base + *(esp + 3) - (u32)running->k_stack_base;
	*(fork->k_stack + 4)	= (u32)fork->k_stack;

	awaken_processes_push(fork);

	return fork_pid;
}

void
exit_user_process(u32 status, u32* esp)
{
	ready_processes.head->status = ZOMBIE;
	ready_processes.head->exit_code = status;
	// TODO: clear mm content
	//kfree(ready_processes.head->k_stack_base);
	
	// Note: we schedule here since user process exits with syscall exit
	// which runs this routine. Schedule triggers restores eflags when switching context.
	schedule(esp);
}


pid_t
create_process(proc_info_t* info)
{
	process_t*	running = ready_processes.head;
	// TODO: group R/W to static variable to surround them with spinlock
	pid_t		pid = new_pid();
	if (!pid) return 0;

	u8*	k_stack_base = (u8*)kmalloc(STACK_SIZE);
	if (!k_stack_base) return 0;

	memset(k_stack_base, 0, STACK_SIZE);

	/* PROCESS PID & STATUS */
	process_t*	p	= (process_t*)k_stack_base;
	process_table[pid - 1]	= p;
	p->pid			= pid;

	/* PROCESS MEMORY MANAGEMENT */
	if (info->type == UPROC)
	{
		p->mm = (mm_t*)((u32)(p->k_stack_base + sizeof(process_t) + 7) & ~3);

		for (u32 i = 0; i < sizeof(mm_t); i++)
		{
			((u8*)p->mm)[i] = 0;
		}
		
		p->mm->dir = vmm_setup_process(info->code_size, info->data_size, info->code, info->data);
		if (!p->mm->dir) return 0; // TODO: cleanup

		p->mm->code_start	= PROCESS_CODE_START;
		p->mm->code_end		= PROCESS_CODE_START + info->code_size;

		p->mm->data_start	= PROCESS_CODE_START + ((info->code_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));
		p->mm->data_end		= p->mm->data_start + info->data_size;

		p->mm->stack		= 0xBFFFFFFF;
		p->mm->stack_base	= PROCESS_STACK_START;
	}

	/* PROCESS KSTACK */
	p->k_stack_base = k_stack_base;

	u32* stk = (u32*)(p->k_stack_base + STACK_SIZE);

	if (info->type == KPROC)
	{
		*(--stk) = pid;
		// Note: Dummy return address (we pretend that it is a simple ret call)
		*(--stk) = 0;
		*(--stk) = (u32)exit_process;
	}
	u32* user_esp = stk;
	
	// Set up interrupt return frame (for iret)
	if (info->type == UPROC)
	{
		*(--stk) = 0x23;			// SS
		*(--stk) = p->mm->stack;		// user esp
	}
	*(--stk) = 0x202;				// eflags (Interrupt flag set)
	*(--stk) = info->type == UPROC ? 0x1B : 0x08;	// CS (0x1B user code segment/ 0x08 for kernel)
	*(--stk) = info->entry;				// eip
	
	// Error code and interrupt number (skipped by add $8, %esp)
	*(--stk) = 0;					// err_code
	*(--stk) = 0;					// s32_no
	
	// General purpose registers (for popa)
	*(--stk) = 0;					// EAX
	*(--stk) = 0;					// ECX
	*(--stk) = 0;					// EDX
	*(--stk) = 0;					// EBX
	*(--stk) = (u32)user_esp;			// ESP (original - points to exit_process for KPROC)
							// User process exits with syscall exit
	*(--stk) = 0;					// EBP
	*(--stk) = 0;					// ESI
	*(--stk) = 0;					// EDI
	
	// Segment selector (for ds restore)
	*(--stk) = info->type == UPROC ? 0x23 : 0x10;	// DS (0x23 user data segment, or 0x10 for kernel)
	
	p->k_stack = stk;

	/* PROCESS RELATIONSHIPS */
	p->parent			= running;
	p->self.data			= &p;
	p->children			= 0;
	p->children_lock.counter	= 0;

	if (running)
	{
		p->sibilings = &running->children;
		spinlock_lock(&running->children_lock);
		single_ll_push(&running->children, &p->self);
		spinlock_unlock(&running->children_lock);
	}

	/* PROCESS SCHEDULING */
	p->next = kernel_process;

	awaken_processes_push(p);

	return pid;
}

static void
ft_kernel_process(void)
{
	asm volatile("sti\n\t");
	while (1)
	{
		asm volatile ("hlt\n\t");
	}
}

void
init_multitasking(void)
{
	// Note: Init kernel process
	proc_info_t	kernel_proc_info=
	{
		KPROC, 0, 0, 0, 0,
		(u32)ft_kernel_process
	};

	create_process(&kernel_proc_info);
	ready_processes.head		= awaken_processes.head;
	ready_processes.tail		= awaken_processes.tail;
	awaken_processes.head		= 0;
	awaken_processes.tail		= 0;
	kernel_process			= ready_processes.head;
	kernel_process->status		= RUNNING;
	
	// Note: Init TSS
	tss.esp0			= (u32)kernel_process->k_stack;
	tss.ss0				= 0x10;
	tss.io_permission_bitmap	= sizeof(tss);

	set_gdt_gate(5, (u32)&tss, sizeof(tss), 0x89, 0);
	tss_flush();

	start_process(kernel_process->k_stack);
}

void
schedule(u32* old_esp)
{
	if (atomic_read(&scheduler_locked)) return;
	
	if (awaken_processes.head && spinlock_try_lock(&awaken_processes_lock))
	{
		awaken_processes.tail->next	= kernel_process;
		ready_processes.tail->next	= awaken_processes.head;
		ready_processes.tail		= awaken_processes.tail;
		awaken_processes.head		= 0;
		awaken_processes.tail		= 0;
		spinlock_unlock(&awaken_processes_lock);
	}
	
	process_t*	old	= ready_processes.head;

	if (!kernel_process
		|| !old
		|| (old == kernel_process && ready_processes.tail == kernel_process))
		return;

	old->k_stack			= old_esp;
	ready_processes.head		= old->next;
	ready_processes.head->status	= RUNNING;
	old->next			= kernel_process;
	
	if (old->status == RUNNING)
	{
		old->status			= READY;
		ready_processes.tail->next	= old;
		ready_processes.tail		= old;
	}

	process_t*	running = ready_processes.head;

	tss.esp0 = (u32)running->k_stack;
	
	if (running->mm)
	{
		switch_process_user(running->k_stack, (u32)running->mm->dir);
	}
	else
	{
		switch_process(running->k_stack);
	}
}
