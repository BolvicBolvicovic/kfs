ENTRY(_start)

SECTIONS
{
	. = 2M;
	.text BLOCK(8K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
	}

	.rodata BLOCK(8K) : ALIGN(4K)
	{
		*(.rodata)
	}

	.data BLOCK(8K) : ALIGN(4K)
	{
		*(.data)
	}

	.bss BLOCK(8K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
}
