OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)
virt = 0xC0000000;
SECTIONS
{
	. = 0x00100000;
	start_kernel = .;
	.multiboot :{
		*(.multiboot)
		*(.boot.text)
	}
	. += virt;
	start_kernel_virt = .;
	.text ALIGN(4K) : AT(ADDR(.text) - virt)
	{
		code = .;
		*(.text)
		*(.rodata)
	}
	.data ALIGN(4K) : AT(ADDR(.data) - virt)
	{
		*(.data)
	}
	.bss ALIGN(4K) : AT(ADDR(.bss) - virt) 
	{
		*(COMMON)
		*(.bss)
		*(.boot.stack)
	}
	end_kernel_virt = .;
    endkernel = . - virt;
}
