#ifndef UTILS_H
#define UTILS_H

inline unsigned char
port_byte_in(unsigned short port)
{
    unsigned char result;
    asm volatile("inb %1, %0" : "=a" (result) : "dN" (port));
    return result;
}

inline unsigned short
port_word_in(unsigned short port)
{
    unsigned short result;
    asm volatile("inw %1, %0" : "=a" (result) : "dN" (port));
    return result;
}

inline unsigned long
port_long_in(unsigned short port)
{
    unsigned long result;
    asm volatile("inl %1, %0" : "=a" (result) : "dN" (port));
    return result;
}

inline void
port_byte_out(unsigned short port, unsigned char data)
{
    asm volatile("outb %1, %0" : : "dN" (port), "a" (data));
}

inline void
port_word_out(unsigned short port, unsigned short data)
{
    asm volatile("outw %1, %0" : : "Nd" (port), "a" (data));
}

inline void
port_long_out(unsigned short port, unsigned long data)
{
    asm volatile("outl %1, %0" : : "Nd" (port), "a" (data));
}

inline void
insl(unsigned short port, void *buffer, unsigned int count)
{
    asm volatile (
        "rep insl"                // Repeat the 'insl' instruction 'count' times
        : "+D"(buffer), "+c"(count)  // '+D' is the destination (EDI in x86), '+c' is the counter (ECX in x86)
        : "d"(port)               // 'd' is the source port (DX in x86)
        : "memory"                // Inform the compiler that memory is affected
    );
}

#define PCI_CONF_ENABLE			0x80000000
#define PCI_CONF_PORT_ADDR		0xCF8
#define PCI_CONF_PORT_DATA		0xCFC
#define PCI_CONF_VID_OFFSET		0x00
#define PCI_CONF_DID_OFFSET		0x02
#define PCI_CONF_CMDR_OFFSET	0x04
#define PCI_CONF_STATUS_OFFSET	0x06
#define PCI_CONF_BAR0_OFFSET	0x10
#define PCI_CONF_CAP_PTR_OFFSET	0x34
#define DWORD_ALIGN				0xFC

static inline unsigned long
pci_conf_read32(
	unsigned char bus,
	unsigned char device,
	unsigned char func,
	unsigned char offset)
{
	// Note: This is not a mapped address.
	unsigned long	addr = PCI_CONF_ENABLE |
							((unsigned long)bus << 16) |
							((unsigned long)device << 11) |
							((unsigned long)func << 8) |
							(offset & DWORD_ALIGN);
	port_long_out(PCI_CONF_PORT_ADDR, addr);
	return port_long_in(PCI_CONF_PORT_DATA);
}

static inline void
pci_conf_write32(
	unsigned char bus,
	unsigned char device,
	unsigned char func,
	unsigned char offset,
	unsigned long value)
{
	// Note: This is not a mapped address.
	unsigned long	addr = PCI_CONF_ENABLE |
							((unsigned long)bus << 16) |
							((unsigned long)device << 11) |
							((unsigned long)func << 8) |
							(offset & DWORD_ALIGN);
	port_long_out(PCI_CONF_PORT_ADDR, addr);
	port_long_out(PCI_CONF_PORT_DATA, value);
}

static inline unsigned short
pci_conf_read16(
	unsigned char bus,
	unsigned char device,
	unsigned char func,
	unsigned char offset)
{
	unsigned long	data = pci_conf_read32(bus, device, func, offset);
	return (data >> ((offset & 2) * 8)) & 0xFFFF;
}

static inline unsigned char
pci_conf_read8(
	unsigned char bus,
	unsigned char device,
	unsigned char func,
	unsigned char offset)
{
	unsigned long	data = pci_conf_read32(bus, device, func, offset);
	return (data >> ((offset & 3) * 8)) & 0xFF;
}

static inline void
pci_conf_write16(
	unsigned char bus,
	unsigned char device,
	unsigned char func,
	unsigned char offset,
	unsigned short value)
{
	unsigned long	data = pci_conf_read32(bus, device, func, offset);
	unsigned short	shift = (offset & 2) * 8;
	data &= ~((unsigned long)0xFFFF << shift);
	data |= ((unsigned long)value << shift);
	pci_conf_write32(bus, device, func, offset, data);
}

static inline void
pci_conf_write8(
	unsigned char bus,
	unsigned char device,
	unsigned char func,
	unsigned char offset,
	unsigned char value)
{
	unsigned long	data = pci_conf_read32(bus, device, func, offset);
	unsigned short	shift = (offset & 3) * 8;
	data &= ~((unsigned long)0xFF << shift);
	data |= ((unsigned long)value << shift);
	pci_conf_write32(bus, device, func, offset, data);
}

#endif
