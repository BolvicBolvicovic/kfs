OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)
phys = 0x00100000;
virt = 0xC0000000;
SECTIONS
{
	.gdt 0x00000800 :
    {
		KEEP(*(.gdt))
	}
    . = ALIGN(0x1000);
	.text virt : AT(phys)
	{
        code = .;
		*(.multiboot)
		*(.text)
		*(.rodata)
        . = ALIGN(0x1000);
	}
	.data (virt + (data - code)) : AT(phys + (data - code))
	{
        data = .;
		*(.data)
        . = ALIGN(0x1000);
	}

	.bss (virt + (bss - code)) : AT(phys + (bss - code))
	{
        bss = .;
		*(.bss)
        . = ALIGN(0x1000);
        ebss = .;
        endkernel = .;
	}
}
