ENTRY(_start)

SECTIONS
{
	. = 2M;
	.text BLOCK(64K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
	}

	.rodata BLOCK(64K) : ALIGN(4K)
	{
		*(.rodata)
	}

	.data BLOCK(64K) : ALIGN(4K)
	{
		*(.data)
	}

	.bss BLOCK(64K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
}
