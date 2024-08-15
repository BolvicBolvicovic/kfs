OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)
phys = 0x00100000;
SECTIONS
{
	. = 0x00000800;
	.gdt : {
		KEEP(*(.gdt))
	}
	.text phys : AT(phys)
	{
        code = .;
		*(.multiboot)
		*(.text)
		*(.rodata)
        . = ALIGN(0x1000);
	}
	.data : AT(phys + (data - code))
	{
        data = .;
		*(.data)
        . = ALIGN(0x1000);
	}

	.bss : AT(phys + (bss - code))
	{
        bss = .;
		*(.bss)
        . = ALIGN(0x1000);
        ebss = .;
        endkernel = .;
	}
}
