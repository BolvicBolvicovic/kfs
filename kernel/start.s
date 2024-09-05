.extern	kernel_main
.global	_start
.global stack_bottom
.global stack_top

.section	.multiboot
.align	4
.long	0x1BADB002			# MAGIC
.long	(1 << 0) | (1 << 1)	# FLAGS
.long	-(0x1BADB002 + ((1 << 0) | (1 << 1))) # CHECKSUM

.section	.bss
.align		16
stack_bottom:
.skip       0x10000
stack_top:

.section	.text
_start:
	mov     $stack_top, %esp
    pushl   $0
    popf
    pushl   %ebx
    pushl   %eax
	cli
	call	kernel_main
	jmp	.
