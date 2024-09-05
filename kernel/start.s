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

# 4KB align
.set PAGE_DIR,           0x9C000
.set PAGE_TABLE_0,       0x9D000
.set PAGE_TABLE_768,     0x9E000
.set PAGE_TABLE_ENTRIES, 1024
.set PRIV,               3 # Page present and writable 0b11

.section .multiboot, "ax", @progbits
.align	4
.long	MB_MAGIC
.long	MB_FLAGS
.long	MB_CHECKSUM


.section .boot.text, "ax", @progbits
_start:
    mov $0x10, %ax # Set data segment to data selector
    mov %ax, %ds    
    mov %ax, %ss
    mov %ax, %es
    mov stack_top, %esp
    pusha
    
    mov $PAGE_TABLE_0, %eax
    mov $(0x0 | PRIV), %ebx
    mov $PAGE_TABLE_ENTRIES, %ecx
.loop:
    movl %ebx, (%eax)
    add  $4, %eax
    add  $0x1000, %ebx
    loop .loop

    mov $(PAGE_TABLE_0 | PRIV), %eax
    movl %eax, (PAGE_DIR)

    mov $(PAGE_TABLE_768 | PRIV), %eax
    movl %eax, PAGE_DIR + (768 * 4)

    mov $PAGE_DIR, %eax
    mov %eax, %cr3

    mov %cr0, %eax
    or  $0x80000000, %eax
    mov %eax, %cr0

    mov $PAGE_TABLE_768, %eax
    mov $(0x100000 | PRIV), %ebx
    mov $PAGE_TABLE_ENTRIES, %ecx
.loop2:
    movl %ebx, (%eax)
    add  4, %eax
    add  0x1000, %ebx
    loop .loop2

    popa

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
    
.section	.bss
.align		32
stack_bottom:
.skip       0x10000
stack_top:
