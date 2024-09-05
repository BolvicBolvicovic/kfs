.global stack_top
.global stack_bottom

.set ALIGN,		(1 << 0)
.set MEMINFO,     	(1 << 1)
.set MB_MAGIC,	  	0x1BADB002
.set MB_FLAGS,	  	(ALIGN | MEMINFO)
.set MB_CHECKSUM, 	-(MB_MAGIC + MB_FLAGS)

.extern start_kernel 
.extern endkernel
.extern start_kernel_virt
.extern end_kernel_virt 

.set PAGE_TABLE_ENTRIES, 1024
.set PRIV,               3 # Page present and writable 0b11
.set PAGE_SIZE,          0x1000
.set KERNEL_VIRT_ADDR, 0xC0000000

.section .multiboot, "aw", @progbits
.align	4
.long	MB_MAGIC
.long	MB_FLAGS
.long	MB_CHECKSUM


.section .boot.stack, "aw", @nobits
stack_bottom:
.skip       0x10000
stack_top:

.section .bss, "aw", @nobits
.align 0x1000
boot_page_dir:
.skip 0x1000
# If Kernel grows over 3MB, further page table will be needed
boot_page_table0:
.skip 0x1000

.section .boot.text, "ax", @progbits
.global	_start
#.type _start, @function
_start:
    movl $(boot_page_table0 - KERNEL_VIRT_ADDR), %edi
    movl $0, %esi
    movl $1024, %ecx
1:
    # Select Kernel
    cmpl $start_kernel, %esi
    jl   2f
    cmpl $endkernel, %esi
    jge  3f
    # Map addr as present and writable
    movl %esi, %edx
    orl  $PRIV, %edx
    movl %edx, (%edi)
2:
    addl $PAGE_SIZE, %esi
    addl $4, %edi # Move to next entry in table (32 bits entry)
    loop 1b
3:
    # Map the page table to both address 0x00000000 and 0xC0000000 because enabling paging does not change the next instruction that continues to be physical
    movl $(boot_page_table0 - KERNEL_VIRT_ADDR + PRIV), boot_page_dir - KERNEL_VIRT_ADDR
    movl $(boot_page_table0 - KERNEL_VIRT_ADDR + PRIV), boot_page_dir - KERNEL_VIRT_ADDR + 768 * 4

    # Set page dir to cr3
    movl $boot_page_dir - KERNEL_VIRT_ADDR, %ecx
    # Enable Paging and write-protect bit (supervisor cannot write on read-only pages)
    movl %cr0, %ecx
    or   $0x80010000, %ecx
    movl %ecx, %cr0

    lea 4f, %ecx
    jmp *%ecx

.section .text
.align 4
# Higher Half Kernel
4:
    # Reload cr3 forces a TLB flush and changes take effect
    movl %cr3, %ecx
    movl %ecx, %cr3

    mov  $stack_top, %esp

    pushl   %esp
    pushl   %ebx
    pushl   %eax
    .extern    kernel_main
    call       kernel_main
    cli
1:
    hlt
    jmp 1b
