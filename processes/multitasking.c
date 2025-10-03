#include "processes.h"
#include "../memory/vmm/vmm.h"

#define PT_SIZE 64
#define STACK_SIZE (0x1000 * 2)

// TODO: Fix memcpy so that it returns a pointer
extern void		memcpy(void* d, const void* s, uint32_t n);
extern void*	memset(void* s, int c, uint32_t n);

// Note: These are from switch.s
extern void		switch_process(uint32_t** old, uint32_t* new);
extern void		start_process(uint32_t* new);
extern void		save_child_registers(uint32_t** child);


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
fork_process(void)
{
	process*	fork = get_next_process_space();
	uint32_t	fork_pid = new_pid();

	if (!current_process || !fork || !fork_pid)
	{
		return 0;
	}

	fork->pid 				= fork_pid;
	fork->uid 				= current_process->uid;
	fork->parent			= current_process->pid;

	fork->status			= READY;
	fork->signals			= 0;
	fork->next				= 0;

	fork->stack_base		= kmalloc(STACK_SIZE);
	fork->stack				= fork->stack_base + ((uint32_t)current_process->stack - (uint32_t)current_process->stack_base);
	memcpy(fork->stack_base, current_process->stack_base, STACK_SIZE);

	save_child_registers(&fork->stack);

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
	//kfree(current_process->heap);
	kfree(current_process->stack_base);
	schedule();
}


pid_t
create_process(void (*entry)(void))
{
	process*	p = get_next_process_space();
	pid_t		pid = new_pid();
	
	if (!p || !pid) return 0;

	p->pid = pid;
	p->stack_base = kmalloc(STACK_SIZE);
	//p->heap = kmalloc(4096);

	memset(p->stack_base, 0, STACK_SIZE);
	//memset(p->heap, 0, 4096);

	uint32_t* stk = (uint32_t*)(p->stack_base + STACK_SIZE);
	
	// Put exit_process as return address on the user stack
	// (so when entry() returns, it will "ret" to exit_process)
	*(--stk) = (uint32_t)exit_process;
	
	uint32_t* user_esp = stk;  // This is where the user stack pointer should be
	
	// Set up interrupt return frame (for iret)
	//*(--stk) = 0x10;                    // SS (0x23 user data segment/ 0x10 for kernel)
	//*(--stk) = (uint32_t)user_esp;      // useresp (points to exit_process on user stack)
	*(--stk) = 0x200;                   // eflags (IF flag set to enable interrupts)
	*(--stk) = 0x08;                    // CS (0x1B user code segment/ 0x08 for kernel)
	*(--stk) = (uint32_t)entry;         // eip (entry point)
	
	// Error code and interrupt number (skipped by add $8, %esp)
	*(--stk) = 0;                       // err_code
	*(--stk) = 0;                       // int_no
	
	// General purpose registers (for popa)
	*(--stk) = 0;                       // EAX
	*(--stk) = 0;                       // ECX
	*(--stk) = 0;                       // EDX
	*(--stk) = 0;                       // EBX
	*(--stk) = (uint32_t)user_esp;      // ESP (original - points to exit_process)
	*(--stk) = 0;                       // EBP
	*(--stk) = 0;                       // ESI
	*(--stk) = 0;                       // EDI
	
	// Segment selector (for ds restore)
	*(--stk) = 0x10;                    // DS (0x23 user data segment, or 0x10 for kernel)
	
	p->stack = stk;
	p->status = READY;
	p->next = 0;
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
	while (1)
	{
		asm volatile ("hlt\n\t");
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
