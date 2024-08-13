.extern	kernel_main
.global	_start
.set	MB_MAGIC, 0x1BADB002
.set	MB_FLAGS, (1 << 0) | (1 << 1)
.set	MB_CHECKSUM, -(MB_MAGIC + MB_FLAGS)
		
.section	.multiboot
.align	4
.long	MB_MAGIC
.long	MB_FLAGS
.long	MB_CHECKSUM

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
