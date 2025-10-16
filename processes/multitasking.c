#include "processes.h"

#define PT_SIZE		1024
#define STACK_SIZE	0x2000

extern void*	memcpy(void* d, const void* s, uint32_t n);
extern void*	memset(void* s, uint8_t c, uint32_t n);
extern void		switch_dir(uint32_t dir);

// Note: These are from switch.s
extern void		switch_process(uint32_t* new);
extern void		switch_process_user(uint32_t* new_stack, uint32_t dir);
extern void		start_process(uint32_t* new);
extern void		tss_flush(void);

static tss_t	tss;
// TODO: Use hash table instead with pid as index
static process*	process_table[PT_SIZE] = {0};
static process*	kernel_process = 0;
static process*	current_process = 0;
static process*	tail_process = 0;

static inline pid_t
new_pid(void)
{
	static uint32_t g_pid_count	= 1;

	uint32_t		start_pid	= g_pid_count;

	do
	{
		if (!process_table[g_pid_count - 1] || process_table[g_pid_count - 1]->status == ZOMBIE)
		{
			pid_t	pid = g_pid_count;
            g_pid_count = (pid % (PT_SIZE - 1)) + 1;
			return pid;
		}
		
		g_pid_count = (g_pid_count % (PT_SIZE - 1)) + 1;
	} while (g_pid_count != start_pid);
	
	return 0;
}

static inline process*
get_process(pid_t p)
{
	if (p <= 1 || p > PT_SIZE - 1)
	{
		return 0;
	}

	return process_table[p - 1];
}

static void
exit_process(uint32_t pid)
{
	process_table[pid - 1]->status = ZOMBIE;
	//kfree(current_process->k_stack_base);
	for (;;)
	{
		asm volatile ("hlt\n\t");
	}
}


int
queue_signal(pid_t p, uint32_t sig)
{
	if (!p || p > PT_SIZE - 1)
	{
		return 0;
	}

	process_table[p - 1]->pending_signals |= sig;
	return 1;
}

int
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
fork_process(uint32_t* esp)
{
	// TODO: Handle user process
	pid_t		fork_pid = new_pid();
	if (!fork_pid) return 0;

	uint8_t*	k_stack_base = (uint8_t*)kmalloc(STACK_SIZE);
	if (!k_stack_base) return 0;

	memset(k_stack_base, 0, STACK_SIZE);

	process*	fork = (process*)k_stack_base;
	process_table[fork_pid - 1] = fork;

	if (!k_stack_base)
	{
		return 0;
	}

	fork->pid 				= fork_pid;
	fork->uid 				= current_process->uid;
	fork->parent			= (uint32_t)current_process;

	fork->status			= READY;
	fork->pending_signals	= 0;
	fork->next				= (uint32_t)kernel_process;
	fork->k_stack_base		= k_stack_base;
	uint32_t used_k_stack	= (uint32_t)(current_process->k_stack_base + STACK_SIZE) - (uint32_t)esp;
	fork->k_stack			= (uint32_t*)(fork->k_stack_base + STACK_SIZE - used_k_stack);

	memcpy(fork->k_stack, esp, used_k_stack);

	if (!current_process->mm)
	{
		*(uint32_t*)(fork->k_stack_base + STACK_SIZE - 4) = fork_pid; // Set exit_process input
	}
	else
	{
		// TODO: check how to handle page directory for parent/child
		fork->mm = memcpy((void*)((uint32_t)(fork->k_stack_base + sizeof(process) + 7) & ~3), current_process->mm, sizeof(mm_t));
	}

	*(fork->k_stack + 8)	= 0; // Set eax to 0
	*(fork->k_stack + 3)	= (uint32_t)fork->k_stack_base + *(esp + 3) - (uint32_t)current_process->k_stack_base; // Set ebp
	*(fork->k_stack + 4)	= (uint32_t)fork->k_stack; // Set esp

	tail_process->next		= (uint32_t)fork;
	tail_process			= fork;

	return fork_pid;
}

void
exit_user_process(uint32_t status, uint32_t* esp)
{
	current_process->status = ZOMBIE;
	current_process->exit_code = status;
	// TODO: clear mm content
	//kfree(current_process->k_stack_base);
	schedule(esp);
}


pid_t
create_process(proc_info_t* info)
{
	// Note: Disable interruption to avoid race condition when creating a process.
	//asm volatile ("cli;");
	pid_t		pid = new_pid();
	if (!pid) return 0;

	uint8_t*	k_stack_base = (uint8_t*)kmalloc(STACK_SIZE);
	if (!k_stack_base) return 0;

	memset(k_stack_base, 0, STACK_SIZE);

	/* PROCESS PID & STATUS */
	process*	p = (process*)k_stack_base;
	process_table[pid - 1] = p;
	p->pid = pid;
	p->status = READY;

	/* PROCESS KSTACK */
	p->k_stack_base = k_stack_base;

	uint32_t* stk = (uint32_t*)(p->k_stack_base + STACK_SIZE);

	if (info->type == KPROC)
	{
		*(--stk) = pid;
		// Note: Dummy
		*(--stk) = 0;
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
		p->mm = (mm_t*)((uint32_t)(p->k_stack_base + sizeof(process) + 7) & ~3);

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
	p->parent = (uint32_t)current_process;

	/* PROCESS SCHEDULING */
	p->next = (uint32_t)kernel_process;
	if (!current_process)
	{
		current_process = p;
	}
	else
	{
		tail_process->next = (uint32_t)p;
	}
	tail_process = p;

	// Note: Re-enable interrupion.
	//asm volatile ("sti;");

	return pid;
}

static void
ft_kernel_process(void)
{
    //asm volatile("sti\n\t");
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
		(uint32_t)ft_kernel_process
	};
	create_process(&kernel_proc_info);
	kernel_process = current_process;
	// Note: need to do this else next is 0
	kernel_process->next = (uint32_t)kernel_process;
	kernel_process->status = RUNNING;
	
	// Note: Init TSS
	tss.esp0 = (uint32_t)kernel_process->k_stack;
	tss.ss0 = 0x10;
	tss.io_permission_bitmap = sizeof(tss);

	set_gdt_gate(5, (uint32_t)&tss, sizeof(tss), 0x89, 0);
	tss_flush();

	start_process(kernel_process->k_stack);
}

void
schedule(uint32_t* old_esp)
{
	if (!kernel_process || !current_process || (current_process == kernel_process && tail_process == kernel_process)) return;

	process* prev = current_process;
	
	prev->k_stack = old_esp;
	current_process = (process*)current_process->next;
	current_process->status = RUNNING;
	
	if (prev->status == RUNNING)
	{
		prev->next = (uint32_t)kernel_process;
		prev->status = READY;
		tail_process->next = (uint32_t)prev;
		tail_process = prev;
	}

	tss.esp0 = (uint32_t)current_process->k_stack;
	
	if (current_process->mm)
	{
		switch_process_user(current_process->k_stack, (uint32_t)current_process->mm->dir);
	}
	else
	{
		switch_process(current_process->k_stack);
	}
}
