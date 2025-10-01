#include "processes.h"
#include "../memory/vmm/vmm.h"

#define PT_SIZE 64

extern void*   memset(void* s, int c, uint32_t n);

void exit_process(void);

// TODO: Use hash table instead with pid as index
static process		process_table[PT_SIZE] = {0};
static process*	current_process = 0;
static process*	head_process = 0;
static process*	tail_process = 0;

static inline pid_t
new_pid(void)
{
	static uint32_t g_pid_count = 0;
	g_pid_count++;
	if (g_pid_count == PT_SIZE - 1 && process_table[0].status == ZOMBIE)
	{
		g_pid_count = 1;
	}
	else if (g_pid_count == PT_SIZE - 1 && process_table[0].status != ZOMBIE)
	{
		g_pid_count--;
		return 0;
	}
	return g_pid_count;
}

static inline process*
get_next_process_space(void)
{
	for (uint32_t i = 0; i < PT_SIZE; i++)
	{
		if (process_table[i].status == ZOMBIE)
		{
			return &process_table[i];
		}
	}

	return 0;
}

static inline process*
get_process(pid_t p)
{
	if (!p || p > PT_SIZE - 1)
	{
		return 0;
	}

	return &process_table[p - 1];
}

static inline void
switch_process(process* old, process* new)
{
    asm volatile (
        // --- Save old registers ---
        "movl %%eax, 0(%0)\n\t"
        "movl %%ebx, 4(%0)\n\t"
        "movl %%ecx, 8(%0)\n\t"
        "movl %%edx, 12(%0)\n\t"
        "movl %%esi, 16(%0)\n\t"
        "movl %%edi, 20(%0)\n\t"
        "movl %%esp, 24(%0)\n\t"
        "movl %%ebp, 28(%0)\n\t"

        // Save eflags
        "pushf\n\t"
        "popl 36(%0)\n\t"

        // Save CR3
        "movl %%cr3, %%eax\n\t"
        "movl %%eax, 40(%0)\n\t"

        // Save EIP: tricky part
        "call 1f\n\t"        // push return address
        "1: popl 32(%0)\n\t" // pop return address into old->regs.eip

        // --- Load new registers ---
        "movl 40(%1), %%eax\n\t"   // load CR3
        "movl %%eax, %%cr3\n\t"

        "movl 0(%1), %%eax\n\t"
        "movl 4(%1), %%ebx\n\t"
        "movl 8(%1), %%ecx\n\t"
        "movl 12(%1), %%edx\n\t"
        "movl 16(%1), %%esi\n\t"
        "movl 20(%1), %%edi\n\t"
        "movl 28(%1), %%ebp\n\t"

        "movl 24(%1), %%esp\n\t"   // switch to new stack

        // Restore eflags
        "pushl 36(%1)\n\t"
        "popf\n\t"

        // Jump to new EIP
        "jmp *32(%1)\n\t"
        :
        : "r"(&(old->regs)), "r"(&(new->regs))
        : "memory"
    );
}

static inline void
start_process(uint32_t* new)
{
    asm volatile (
        // --- Load new registers ---
        "movl 40(%0), %%eax\n\t"   // load CR3
        "movl %%eax, %%cr3\n\t"

        "movl 0(%0), %%eax\n\t"
        "movl 4(%0), %%ebx\n\t"
        "movl 8(%0), %%ecx\n\t"
        "movl 12(%0), %%edx\n\t"
        "movl 16(%0), %%esi\n\t"
        "movl 20(%0), %%edi\n\t"
        "movl 28(%0), %%ebp\n\t"

        "movl 24(%0), %%esp\n\t"   // switch to new stack

        // Restore eflags
        "pushl 36(%0)\n\t"
        "popf\n\t"

        // Jump to new EIP
        "jmp *32(%0)\n\t"
        :
        : "r"(&(new->regs))
        : "memory"
    );
}

int
queue_signal(pid_t p, uint32_t sig)
{
	if (!p || p > PT_SIZE - 1)
	{
		return 0;
	}

	process_table[p - 1].signals |= sig;
	return 1;
}

int
update_status(pid_t p, process_status s)
{
	if (!p || p > PT_SIZE - 1)
	{
		return 0;
	}

	process_table[p - 1].status = s;
	return 1;
}

pid_t
fork_process(pid_t p)
{
	process*	current = get_process(p);
	process*	fork = get_next_process_space();
	uint32_t	fork_pid = new_pid();

	if (!current || !fork || !fork_pid)
	{
		return 0;
	}

	//fork->pid 				= fork_pid;
	fork->uid 				= current->uid;
	fork->status			= current->status;
	fork->signals			= current->signals;
	fork->regs				= current->regs;
	fork->next				= 0;
	//fork->parent			= current->parent;
	for (uint32_t i = 0; i < 4096; i++)
	{
		// TODO: copy 4 by 4
		fork->stack[i]		= current->stack[i];
		fork->heap[i] 		= current->heap[i];
		//if (i > 31) continue;
		//fork->fds[32]		= current->fds[i];
		//if (i > 15) continue;
		//fork->children[16]	= current->children[i];
	}

	if (tail_process) {
		tail_process->next = (uint32_t)fork;
	}

	return fork_pid;
}

void
exit_process(void)
{
	current_process->status = ZOMBIE;
	schedule();
}

pid_t
create_process(void (*entry)(void))
{
	process*	p = get_next_process_space();
	pid_t		pid = new_pid();
	
	if (!p || !pid) return 0;

	memset(p->stack, 0, 4096);
	
	uint32_t stk		= (uint32_t)(p->stack) + 4096;
	*(uint32_t*)(--stk)	= exit_process;
	p->regs.esp			= stk;
	p->regs.eip			= (uint32_t)entry;
	p->regs.eax			= p->regs.ebx = p->regs.ecx = p->regs.edx = 0;
	p->regs.esi			= p->regs.edi = p->regs.ebp = 0;
	p->status			= READY;
	p->next				= 0;

	if (!head_process)
	{
		head_process = p;
	}
	else
	{
		tail_process->next = (uint32_t)p;
	}
	tail_process = p;

	return pid;
}

static void
kernel_process(void)
{
	int	i = 0;
	while (1)
	{
		if (!(i % 100000))
		{
			yield();
		}
		i++;
		if (i > 1000000) i = 0;
	}
}


void
init_multitasking(void)
{
	if (!current_process)
	{
		create_process(kernel_process);
		current_process = head_process;
		
		head_process = (process*)current_process->next;
		if (!head_process)
		{
			tail_process = 0;
		}
		
		current_process->next = 0;
		current_process->status = RUNNING;
	}
}

void
schedule(void)
{
	if (!head_process) return;

	process* prev = current_process;
	
	current_process = head_process;
	head_process = (process*)current_process->next;
	current_process->next = 0;
	current_process->status = RUNNING;
	
	if (prev && prev->status == RUNNING)
	{
		prev->status = READY;
		if (!head_process)
		{
			head_process = prev;
			tail_process = prev;
		}
		else
		{
			tail_process->next = (uint32_t)prev;
			tail_process = prev;
		}
		prev->next = 0;
	}
	
	if (!head_process)
	{
		tail_process = 0;
	}
	
	if (prev)
	{
		switch_process(prev, current_process);
	}
	else
	{
		start_process(current_process);
	}
}

void
yield(void)
{
	schedule();
}
