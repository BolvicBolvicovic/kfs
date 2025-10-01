#include "processes.h"
#include "../memory/vmm/vmm.h"

#define PT_SIZE 64

extern void*	kmalloc(uint32_t size);
extern void		kfree(void* ptr);
extern void*	memset(void* s, int c, uint32_t n);
extern p_dir*	vmm_get_dir(void);

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

static inline void
switch_process(process* old, process* new)
{
	asm volatile (
        // Save using FIRST pointer (don't modify it)
        "movl %%eax, 0(%0)\n\t"
        "movl %%ebx, 4(%0)\n\t"
        "movl %%ecx, 8(%0)\n\t"
        "movl %%edx, 12(%0)\n\t"
        "movl %%esi, 16(%0)\n\t"
        "movl %%edi, 20(%0)\n\t"

        // Use EBX for temps (we already saved it)
        "leal 4(%%esp), %%ebx\n\t"
        "movl %%ebx, 24(%0)\n\t"

        "movl %%ebp, 28(%0)\n\t"

        "movl (%%esp), %%ebx\n\t"
        "movl %%ebx, 32(%0)\n\t"

        "pushf\n\t"
        "popl %%ebx\n\t"
        "movl %%ebx, 36(%0)\n\t"

        "movl %%cr3, %%ebx\n\t"
        "movl %%ebx, 40(%0)\n\t"

        // Load using SECOND pointer - save it in EBX first!
        "movl %1, %%ebx\n\t"           // EBX = &new->regs

        "movl 40(%%ebx), %%eax\n\t"    // Load CR3
        "movl %%eax, %%cr3\n\t"

        "pushl 36(%%ebx)\n\t"          // Load eflags
        "popf\n\t"

        "movl 28(%%ebx), %%ebp\n\t"    // Load EBP
        "movl 24(%%ebx), %%esp\n\t"    // Load ESP
        "pushl 32(%%ebx)\n\t"          // Push EIP

        // Now restore registers (EBX last since it's our pointer)
        "movl 0(%%ebx), %%eax\n\t"
        "movl 8(%%ebx), %%ecx\n\t"
        "movl 12(%%ebx), %%edx\n\t"
        "movl 16(%%ebx), %%esi\n\t"
        "movl 20(%%ebx), %%edi\n\t"
        "movl 4(%%ebx), %%ebx\n\t"     // Load EBX last

        "ret\n\t"                      // Jump to pushed EIP
        :
        : "r"(&(old->regs)), "r"(&(new->regs))
        : "memory"
    );
}

static inline void
start_process(process* new)
{
    asm volatile (
        // Load using SECOND pointer - save it in EBX first!
        "movl %0, %%ebx\n\t"           // EBX = &new->regs

        "movl 40(%%ebx), %%eax\n\t"    // Load CR3
        "movl %%eax, %%cr3\n\t"

        "pushl 36(%%ebx)\n\t"          // Load eflags
        "popf\n\t"

        "movl 28(%%ebx), %%ebp\n\t"    // Load EBP
        "movl 24(%%ebx), %%esp\n\t"    // Load ESP
        "pushl 32(%%ebx)\n\t"          // Push EIP

        // Now restore registers (EBX last since it's our pointer)
        "movl 0(%%ebx), %%eax\n\t"
        "movl 8(%%ebx), %%ecx\n\t"
        "movl 12(%%ebx), %%edx\n\t"
        "movl 16(%%ebx), %%esi\n\t"
        "movl 20(%%ebx), %%edi\n\t"
        "movl 4(%%ebx), %%ebx\n\t"     // Load EBX last

        "ret\n\t"                      // Jump to pushed EIP
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
	kfree(current_process->stack);
	schedule();
}

pid_t
create_process(void (*entry)(void))
{
	process*	p = get_next_process_space();
	pid_t		pid = new_pid();
	
	if (!p || !pid) return 0;

	p->stack = kmalloc(4096);
	p->heap = kmalloc(4096);

	memset(p->stack, 0, 4096);
	memset(p->heap, 0, 4096);
	
	uint32_t*		stk	= (uint32_t*)((uint32_t)(p->stack) + 4096);
	*(--stk)			= (uint32_t)exit_process;
	p->regs.esp			= (uint32_t)stk;
	p->regs.eip			= (uint32_t)entry;
	p->regs.eax			= p->regs.ebx = p->regs.ecx = p->regs.edx = 0;
	p->regs.esi			= p->regs.edi = p->regs.ebp = 0;
	// Note: Enable interrupts (IF flag)
	p->regs.eflags      = 0x200;
	p->regs.cr3			= (uint32_t)vmm_get_dir();
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
		start_process(current_process);
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
			tail_process = 0;
		}
		else
		{
			tail_process->next = (uint32_t)prev;
			tail_process = prev;
		}
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
    //uint32_t esp_now;
    //asm volatile("movl %%esp, %0" : "=r"(esp_now));
    //printf("yield() called, current ESP=%d\n", esp_now);

	schedule();
}
