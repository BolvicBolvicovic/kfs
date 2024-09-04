.global	_start
.extern endkernel

.set ALIGN,		(1 << 0)
.set MEMINFO,     	(1 << 1)
.set MB_MAGIC,	  	0x1BADB002
.set MB_FLAGS,	  	(ALIGN | MEMINFO)
.set MB_CHECKSUM, 	-(MB_MAGIC + MB_FLAGS)
.set KERNEL_VIRT_ADDR,  0xC0000000
.set KERNEL_PHYS_ADDR,  0x00100000
.set KERNEL_PAGE_NUMBER, (KERNEL_VIRT_ADDR >> 22)

.section .multiboot
.align	4
.long	MB_MAGIC
.long	MB_FLAGS
.long	MB_CHECKSUM


.section .boot.text
_start:
    mov $(boot_page_table1 - KERNEL_VIRT_ADDR), %edi
    xor %esi, %esi
    mov $1023, %ecx
1:
    cmp $0, %esi
    jl  2f
    cmp $(endkernel - KERNEL_VIRT_ADDR), %esi
    jge 3f

    mov %esi, %edx
    or  0x003, %edx
    mov %edx, (%edi)
2:
    add $0x1000, %esi
    add $4, %edi
    loop 1b
3:
    movl $(boot_page_table1 - KERNEL_VIRT_ADDR + 0x003), (boot_page_dir - KERNEL_VIRT_ADDR)
    movl $(boot_page_table1 - KERNEL_VIRT_ADDR + 0x003), (boot_page_dir - KERNEL_VIRT_ADDR + 768 * 4)

    movl (boot_page_dir - KERNEL_VIRT_ADDR), %ecx
    mov  %ecx, %cr3

    mov %cr0, %ecx
    orl $0x80010000, %ecx
    mov %ecx, %cr0

    lea (high_half), %ecx
    jmp %ecx

.section .text
.align 4
high_half:
    movl    $0, (boot_page_dir)
    invlpg  (0)
    mov     stack_top, %esp

    pushl   %esp
    pushl   %ebx
    pushl   %eax
    cli
    .extern    kernel_main
    call       kernel_main
halt:
    hlt
    jmp halt
    
.section	.data
.align		0x1000
boot_page_dir:
.skip 0x1000
boot_page_table1:
.skip 0x1000

.section	.bss
.align		32
stack_bottom:
.skip       0x10000
stack_top:
