#include "descriptor.h"

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
    printf("Number: %d | Message: %s\n", r->int_no, exception_msg[r->int_no]);
    uint32_t* cp_stack = (uint32_t*)pmm_alloc_block();
    for (uint32_t* ptr = (uint32_t*)stack; *ptr && ptr < (uint32_t*)stack + 0x1000; ptr++) {
	*cp_stack++ = *ptr;
    }
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

void	isr_handler(registers_t* r) {
    panic(r);
}

void isr_install() {
    init_gdt();
    init_idt();
}
