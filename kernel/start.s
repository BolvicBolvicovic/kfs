.extern	kernel_main
.global	_start
		
.section	.multiboot
.align	4
.long	0x1BADB002			# MAGIC
.long	(1 << 0) | (1 << 1)	# FLAGS
.long	-(0x1BADB002 + ((1 << 0) | (1 << 1))) # CHECKSUM

.section	.bss
.align		16
stack_bottom:
	.skip	4096
stack_top:

.section	.text
_start:
	mov		$stack_top, %esp
	call	kernel_main
hang:
	cli
	hlt
	jmp		hang
