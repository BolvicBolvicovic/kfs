#include "processes.h"
#include "../memory/vmm/vmm.h"

#define PT_SIZE 64

extern void*	memset(void* s, int c, uint32_t n);
extern void switch_process(uint32_t** old, uint32_t* new);
extern void start_process(uint32_t* new);


void exit_process(void);

// TODO: Use hash table instead with pid as index
static process	process_table[PT_SIZE] = {0};
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

	if (tail_process)
	{
		tail_process->next = (uint32_t)fork;
	}
	else if (head_process)
	{
		head_process->next = (uint32_t)fork;
	}
	tail_process = fork;

	return fork_pid;
}

void
exit_process(void)
{
	current_process->status = ZOMBIE;
	kfree(current_process->heap);
	kfree(current_process->stack_base);
	schedule();
}

#define STACK_SIZE (0x1000 * 2)

pid_t
create_process(void (*entry)(void))
{
	process*	p = get_next_process_space();
	pid_t		pid = new_pid();
	
	if (!p || !pid) return 0;

	p->stack_base = kmalloc(STACK_SIZE);
	p->heap = kmalloc(4096);

	memset(p->stack_base, 0, STACK_SIZE);
	memset(p->heap, 0, 4096);

	uint32_t*		stk	= (uint32_t*)(p->stack_base + STACK_SIZE);
	*(--stk)			= (uint32_t)exit_process;
	uint32_t* final_esp = stk;
	*(--stk)			= (uint32_t)entry;
	*(--stk)			= 0x200;			// EFLAGS => Note: Enable interrupts (IF flag)
	// POPF will consume this
	*(--stk)			= 0; 				// EDI
	*(--stk)			= 0; 				// ESI
	*(--stk)			= 0; 				// EBP
	*(--stk)			= 0; 				// EBX
	*(--stk)			= (uint32_t)final_esp;
	*(--stk)			= 0; 				// EDX
	*(--stk)			= 0; 				// ECX
	*(--stk)			= 0; 				// EAX
	// POPA will consume these 8 values
	*(--stk)			= (uint32_t)vmm_get_dir();	// CR3
	// POP will consume CR3
	p->stack			= stk;

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
		start_process(current_process->stack);
	}
}

void
schedule(void)
{
	if (!head_process) return;

	process* prev = current_process;
	
	current_process = head_process;
	head_process = (process*)current_process->next;
	current_process->status = RUNNING;
	
	if (prev && prev->status == RUNNING)
	{
		prev->next = 0;
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
	}
	
	if (prev)
	{
		switch_process(&prev->stack, current_process->stack);
	}
	else
	{
		start_process(current_process->stack);
	}
}

void
yield(void)
{
	schedule();
}
