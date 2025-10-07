#include "processes.h"

#define PT_SIZE		64
#define STACK_SIZE	0x1000

// TODO: Fix memcpy so that it returns a pointer
extern void		memcpy(void* d, const void* s, uint32_t n);
extern void*	memset(void* s, int c, uint32_t n);

// Note: These are from switch.s
extern void		switch_process(uint32_t** old, uint32_t* new);
extern void		start_process(uint32_t* new);
extern void		tss_flush(void);

static tss_t	tss;
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

static void
exit_process(void)
{
	current_process->status = ZOMBIE;
	kfree(current_process->k_stack_base);
	schedule();
}


int
queue_signal(pid_t p, uint32_t sig)
{
	if (!p || p > PT_SIZE - 1)
	{
		return 0;
	}

	process_table[p - 1].pending_signals |= sig;
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
fork_process(uint32_t* esp)
{
	process*	fork = get_next_process_space();
	uint32_t	fork_pid = new_pid();

	if (!current_process || !fork || !fork_pid)
	{
		return 0;
	}

	fork->pid 				= fork_pid;
	fork->uid 				= current_process->uid;
	fork->parent			= current_process;

	fork->status			= READY;
	fork->pending_signals	= 0;
	fork->next				= 0;
	fork->k_stack_base		= kmalloc(STACK_SIZE);
	uint32_t used_k_stack		= (uint32_t)(current_process->k_stack_base + STACK_SIZE) - (uint32_t)esp;
	fork->k_stack				= fork->k_stack_base + STACK_SIZE - used_k_stack;
	memcpy(fork->k_stack, esp, used_k_stack);

	*(fork->k_stack + 8)		= 0; // Set eax to 0
	*(fork->k_stack + 3)		= (uint32_t)fork->k_stack_base + *(esp + 3) - (uint32_t)current_process->k_stack_base; // Set ebp
	*(fork->k_stack + 4)		= (uint32_t)fork->k_stack; // Set esp

	if (tail_process)
	{
		tail_process->next = (uint32_t)fork;
	}
	else if (head_process)
	{
		head_process->next = (uint32_t)fork;
	}
	else
	{
		head_process = fork;
	}
	tail_process = fork;

	return fork_pid;
}

void
exit_user_process(uint32_t status)
{
	current_process->status = ZOMBIE;
	current_process->exit_code = status;
	// TODO: clear mm content
	kfree(current_process->mm);
	kfree(current_process->k_stack_base);
	schedule();
}


pid_t
create_process(proc_info_t* info)
{
	process*	p = get_next_process_space();

	/* PROCESS PID & STATUS */
	pid_t		pid = new_pid();
	if (!p || !pid) return 0;
	p->pid = pid;
	p->status = READY;

	/* PROCESS KSTACK */
	p->k_stack_base = kmalloc(STACK_SIZE);
	if (!p->k_stack_base) return 0;

	for (uint32_t i = 0; i < STACK_SIZE / 4; i++)
	{
		((uint32_t*)p->k_stack_base)[i] = 0;
	}

	uint32_t* stk = (uint32_t*)(p->k_stack_base + STACK_SIZE);

	if (info->type == KPROC)
	{
		*(--stk) = (uint32_t)exit_process;
	}
	uint32_t* user_esp = stk;
	
	// Set up interrupt return frame (for iret)
	if (info->type == UPROC)
	{
		*(--stk) = 0x23;							// SS
		*(--stk) = (uint32_t)user_esp;				// useresp (points to exit_process on user k_stack)
	}
	*(--stk) = 0x202;								// eflags (Interrupt flag set)
	*(--stk) = info->type == UPROC ? 0x1B : 0x08;	// CS (0x1B user code segment/ 0x08 for kernel)
	*(--stk) = info->entry;							// eip
	
	// Error code and interrupt number (skipped by add $8, %esp)
	*(--stk) = 0;									// err_code
	*(--stk) = 0;									// int_no
	
	// General purpose registers (for popa)
	*(--stk) = 0;									// EAX
	*(--stk) = 0;									// ECX
	*(--stk) = 0;									// EDX
	*(--stk) = 0;									// EBX
	*(--stk) = (uint32_t)user_esp;					// ESP (original - points to exit_process)
	*(--stk) = 0;									// EBP
	*(--stk) = 0;									// ESI
	*(--stk) = 0;									// EDI
	
	// Segment selector (for ds restore)
	*(--stk) = info->type == UPROC ? 0x23 : 0x10;	// DS (0x23 user data segment, or 0x10 for kernel)
	
	p->k_stack = stk;

	/* PROCESS MEMORY MANAGEMENT */
	if (info->type == UPROC)
	{
		p->mm = kmalloc(sizeof(mm_t));
		if (!p->mm) return 0; // TODO: cleanup

		for (uint32_t i = 0; i < sizeof(mm_t); i++)
		{
			((uint8_t*)p->mm)[i] = 0;
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

	/* PROCESS RELATIONSHIPS */
	// TODO: add children if any and sibilings
	p->parent = current_process;

	/* PROCESS SCHEDULING */
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
	// Note: Init kernel process
	proc_info_t	kernel_proc_info =
	{
		KPROC, 0, 0, 0, 0,
		(uint32_t)kernel_process
	};
	create_process(&kernel_proc_info);
	current_process = head_process;
	current_process->status = RUNNING;
	head_process = 0;
	tail_process = 0;
	
	// Note: Init TSS
	tss.esp0 = current_process->k_stack;
	tss.ss0 = 0x10;
	tss.io_permission_bitmap = sizeof(tss);

	set_gdt_gate(5, (uint32_t)&tss, sizeof(tss), 0x89, 0);
	tss_flush();


	start_process(current_process->k_stack);
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
		if (current_process->mm)
		{
			vmm_switch_pdir(current_process->mm->dir);
		}
		tss.esp0 = current_process->k_stack;
		switch_process(&prev->k_stack, current_process->k_stack);
	}
	else
	{
		start_process(current_process->k_stack);
	}
}
