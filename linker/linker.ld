ENTRY(_start)

SECTIONS
{
	. = 2M;
	.text BLOCK(128K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
	}

	.rodata BLOCK(128K) : ALIGN(4K)
	{
		*(.rodata)
	}

	.data BLOCK(128K) : ALIGN(4K)
	{
		*(.data)
	}

	.bss BLOCK(128K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
}
