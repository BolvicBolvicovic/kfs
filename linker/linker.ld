OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)
SECTIONS
{
	. = 0x00100000;
	start_kernel = .;
	.multiboot :{
		*(.multiboot)
		*(.boot.text)
	}
	. += 0xC0000000;
	start_kernel_virt = .;
	.text BLOCK(4K) : AT(ADDR(.text) - 0xC0000000) ALIGN(4K) 
	{
		code = .;
		*(.text)
		*(.rodata)
	}
	.data BLOCK(4K) : AT(ADDR(.data) - 0xC0000000) ALIGN(4K) 
	{
		*(.data)
	}
	.bss BLOCK(4K) : AT(ADDR(.bss) - 0xC0000000) ALIGN(4K) 
	{
		*(COMMON)
		*(.bss)
	}
	end_kernel_virt = .;
        endkernel = . - 0xC0000000;
}
