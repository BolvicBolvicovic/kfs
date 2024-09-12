#include "descriptor.h"
#define PAGE_FAULT 14

//INTERUPTION SERVICE ROUTINE

static char*		exception_msg[] = {
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

void panic(registers_t* r) {
    asm volatile("cli");
    uint32_t stack = r->esp;
    if (r->int_no != PAGE_FAULT) printf("Number: %d | Message: %s\n", r->int_no, exception_msg[r->int_no]);
    uint32_t* cp_stack = (uint32_t*)kmalloc(0x1000);
    if (cp_stack == NULL) goto gt_xor;
    for (uint32_t* ptr = (uint32_t*)stack; *ptr && ptr < (uint32_t*)stack + 0x1000; ptr++) {
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

#define PAGE_PRESENT_PROT_VIOLATION 1
#define PAGE_WRITE_ERROR 2
#define PAGE_USERMODE_ERROR 4

void	page_fault_handler(registers_t* r) {
    uint32_t faulting_addr;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_addr));

    if (r->err_code & PAGE_USERMODE_ERROR) printf("Page fault: usermode error ");
    else if (r->err_code & PAGE_PRESENT_PROT_VIOLATION && faulting_addr && faulting_addr < PAGE_SIZE) return;
    else printf("Page fault: kernelmode error ");

    if (!faulting_addr)	printf("dereferencing NULL pointer ");
    else if (r->err_code & PAGE_PRESENT_PROT_VIOLATION) printf("protection violation ");
    else printf("page not present ");

    if (r->err_code & PAGE_WRITE_ERROR) printf("write error\n");
    else printf("read error\n");

    panic(r);
}

void	isr_handler(registers_t* r) {
    if (r->int_no == PAGE_FAULT) page_fault_handler(r);
    else panic(r);
}

void isr_install() {
    init_gdt();
    init_idt();
}
