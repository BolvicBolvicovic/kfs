#include "descriptor.h"
#include <memory/allocators/kmalloc.h>
#include <bits.h>

#define PAGE_FAULT	14
#define INVALID_OPCODE	6

//INTERUPTION SERVICE ROUTINE

static char*		exception_msg[] =
{
	"Division by zero",
	"Debug",
	"Non Maskable Interrupt",
	"Breakpoint",
	"Into Detected Overflow",
	"Out of Bounds",
	"Invalid Opcode",
	"No Coprocessor",
	
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Bad TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"Unknown Interrupt",
	
	"Coprocessor Fault",
	"Alignment Check",
	"Machine Check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

void
panic(registers_t* r)
{
    uint32_t stack = r->esp;
    if (r->int_no != PAGE_FAULT) printf("Number: %d | Message: %s\n", r->int_no, exception_msg[r->int_no]);

    uint32_t* cp_stack = (uint32_t*)kmalloc(0x1000);
    if (cp_stack == 0) goto gt_xor;

    for (uint32_t* ptr = (uint32_t*)stack; *ptr && ptr < (uint32_t*)stack + 0x1000; ptr++)
	{
		*cp_stack++ = *ptr;
    }

gt_xor:
    asm volatile(
	"xor %eax, %eax\n"
	"xor %ebx, %ebx\n"
	"xor %ecx, %ecx\n"
	"xor %edx, %edx\n"
	"xor %edi, %edi\n"
	"xor %esi, %esi\n" 
	"xor %ebp, %ebp\n"
	//"xor %eip, %eip\n" RD ONLY
	"xor %esp, %esp\n"
    );
    asm volatile("hlt");

}

enum
{
	PAGE_PRESENT_PROT_VIOLATION 	= BIT0,
	PAGE_WRITE_ERROR		= BIT1,
	PAGE_USERMODE_ERROR		= BIT2,
};

#define KVIRT ((uint32_t)&start_kernel_virt)

extern uint32_t		start_kernel_virt;
extern void		flush_tlb_entry(uint32_t);
static pt_entry		(*page_tables)[1024] = (uint32_t(*)[1024])RECURSIVE_PAGETABLES_ADDR;

void
page_fault_handler(registers_t* r)
{
	uint32_t faulting_addr;
	asm volatile("mov %%cr2, %0" : "=r" (faulting_addr));

	if (faulting_addr &&
		(r->err_code & PAGE_USERMODE_ERROR) &&
		(r->err_code & PAGE_PRESENT_PROT_VIOLATION) &&
		faulting_addr < KVIRT)
	{
		uint32_t	pd_index	= PAGE_DIR_INDEX(faulting_addr);
		uint32_t	pt_index	= PAGE_TAB_INDEX(faulting_addr);

		page_tables[pd_index][pt_index] |= pmm_alloc_block() | PE_USER;
		flush_tlb_entry((uint32_t)faulting_addr);

		return;
	}

	if (faulting_addr &&
		!(r->err_code & PAGE_USERMODE_ERROR) &&
		(r->err_code & PAGE_PRESENT_PROT_VIOLATION))
	{
		uint32_t	pd_index	= PAGE_DIR_INDEX(faulting_addr);
		uint32_t	pt_index	= PAGE_TAB_INDEX(faulting_addr);

		page_tables[pd_index][pt_index] |= pmm_alloc_block();
		flush_tlb_entry((uint32_t)faulting_addr);

		return;
	}

	if (r->err_code & PAGE_USERMODE_ERROR)
		printf("Page fault: usermode error ");
	else if (r->err_code & PAGE_PRESENT_PROT_VIOLATION && faulting_addr && faulting_addr < PAGE_SIZE)
		return;
	else 
		printf("Page fault: kernelmode error ");
	
	if (!faulting_addr)
		printf("dereferencing 0 pointer ");
	else if (r->err_code & PAGE_PRESENT_PROT_VIOLATION)
		printf("protection violation ");
	else
		printf("page not present ");
	
	if (r->err_code & PAGE_WRITE_ERROR)
		printf("write error\n");
	else
		printf("read error\n");
	
	panic(r);
}

void
isr_handler(registers_t* r)
{
    if (r->int_no == PAGE_FAULT) page_fault_handler(r);
    else panic(r);
}

void
isr_install()
{
    init_gdt();
    init_idt();
}
