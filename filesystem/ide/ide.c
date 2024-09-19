#include "ide.h"

#define CLEAR_TWO_LAST_BITS 0xFFFFFFFC
#define ATA_DEFAULT_BASE_PRIMARY 0x1F0
#define ATA_DEFAULT_CRTL_PRIMARY 0x3F6
#define ATA_DEFAULT_BASE_SECONDARY 0x170
#define ATA_DEFAULT_CRTL_SECONDARY 0x376
#define MASTER_DRIVE 0xA0

channels 	chnls   = {0};
ide_devices 	devices = {0};
uint8_t ide_buf[2048] = {0};
volatile static uint8_t ide_irq_invoked = 0;
static uint8_t atapi_packet[12] = {ATAPI_CMD_READ, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
/*
static inline unsigned char port_byte_in(unsigned short port) {
    unsigned char result;
    asm volatile("inb %1, %0" : "=a" (result) : "dN" (port));
    return result;
}

static inline void port_byte_out(unsigned short port, unsigned char data) {
    asm volatile("outb %1, %0" : : "dN" (port), "a" (data));
}

static inline void insl(unsigned short port, void *buffer, unsigned int count) {
    asm volatile (
        "rep insl"                // Repeat the 'insl' instruction 'count' times
        : "+D"(buffer), "+c"(count)  // '+D' is the destination (EDI in x86), '+c' is the counter (ECX in x86)
        : "d"(port)               // 'd' is the source port (DX in x86)
        : "memory"                // Inform the compiler that memory is affected
    );
}
*/
uint8_t ide_read(uint8_t channel, uint8_t reg) {
	uint8_t result;

	if (reg > 0x07 && reg < 0x0C) // If secondary channel register, 48 bits addressing enabled 0x80
		ide_write(channel, ATA_REG_CONTROL, 0x80 | chnls[channel].nIEN);
	if (reg < 0x08)
		result = port_byte_in(chnls[channel].base + reg);
	else if (reg < 0x0C)
		result = port_byte_in(chnls[channel].base + reg - 0x06);
	else if (reg < 0x0E)
		result = port_byte_in(chnls[channel].crtl + reg - 0x0A);
	else if (reg < 0x16)
		result = port_byte_in(chnls[channel].bmide + reg - 0x0E);
	if (reg > 0x07 && reg < 0x0C) 
		ide_write(channel, ATA_REG_CONTROL, chnls[channel].nIEN);

	return result;
}

void    ide_write(uint8_t channel, uint8_t reg, uint8_t data) {

	if (reg > 0x07 && reg < 0x0C) // If secondary channel register, 48 bits addressing enabled 0x80
		ide_write(channel, ATA_REG_CONTROL, 0x80 | chnls[channel].nIEN);
	if (reg < 0x08)
		 port_byte_out(chnls[channel].base + reg, data);
	else if (reg < 0x0C)
		 port_byte_out(chnls[channel].base + reg - 0x06, data);
	else if (reg < 0x0E)
		 port_byte_out(chnls[channel].crtl + reg - 0x0A, data);
	else if (reg < 0x16)
		 port_byte_out(chnls[channel].bmide + reg - 0x0E, data);
	if (reg > 0x07 && reg < 0x0C) // If secondary channel register, take off the privious flag
		ide_write(channel, ATA_REG_CONTROL, chnls[channel].nIEN);
}

void    ide_read_buffer(uint8_t channel, uint8_t reg, uint32_t buffer, uint32_t quads) {
	if (reg > 0x07 && reg < 0x0C) // If secondary channel register, 48 bits addressing enabled 0x80
		ide_write(channel, ATA_REG_CONTROL, 0x80 | chnls[channel].nIEN);
	asm("pushw %es; pushw %ax; movw %ds, %ax; movw %ax, %es; popw %ax;");
	if (reg < 0x08)
		 insl(chnls[channel].base + reg, (void*)buffer, quads);
	else if (reg < 0x0C)
		 insl(chnls[channel].base + reg - 0x06, (void*)buffer, quads);
	else if (reg < 0x0E)
		 insl(chnls[channel].crtl + reg - 0x0A, (void*)buffer, quads);
	else if (reg < 0x16)
		 insl(chnls[channel].bmide + reg - 0x0E, (void*)buffer, quads);
	asm("popw %es;"); 
	if (reg > 0x07 && reg < 0x0C)
		ide_write(channel, ATA_REG_CONTROL, chnls[channel].nIEN);
}

uint8_t ide_polling(uint8_t channel, uint32_t advanced_check) {
	// (Part I) Delay 400 nsec for BSY to be set
	for (uint8_t i = 0; i < 4; i++) ide_read(channel, ATA_REG_ALTSTATUS);
	// (Part II) Wait for BSY to be cleared
	while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY);
	if (advanced_check) {
		uint8_t state = ide_read(channel, ATA_REG_STATUS);
		if (state & ATA_SR_ERR)    return 2; // Error
		if (state & ATA_SR_DF)     return 1; // Device Fault
		if (!(state & ATA_SR_DRQ)) return 3; // Data RQ not set
	}

	return 0;
}

uint8_t ide_print_error(uint32_t drive, uint8_t err) {
	if (!err) return err;
	printf("IDE:");
	switch (err) {
		case 1:
			printf("Device Fault\n");		
			err = 19;
			break;
		case 2: {
			uint8_t st = ide_read(devices[drive].channel, ATA_REG_ERROR);
      			if (st & ATA_ER_AMNF)   {printf("- No Address Mark Found\n     ");   err = 7;}
      			if (st & ATA_ER_TK0NF)   {printf("- No Media or Media Error\n     ");   err = 3;}
      			if (st & ATA_ER_ABRT)   {printf("- Command Aborted\n     ");      err = 20;}
      			if (st & ATA_ER_MCR)   {printf("- No Media or Media Error\n     ");   err = 3;}
      			if (st & ATA_ER_IDNF)   {printf("- ID mark not Found\n     ");      err = 21;}
      			if (st & ATA_ER_MC)   {printf("- No Media or Media Error\n     ");   err = 3;}
      			if (st & ATA_ER_UNC)   {printf("- Uncorrectable Data Error\n     ");   err = 22;}
      			if (st & ATA_ER_BBK)   {printf("- Bad Sectors\n     ");       err = 13;}
		}
		case 3:
			printf("- Reads Nothing\n");
			err = 23;
			break;		
		case 4:
			printf("- [%s %s] %s\n",
      				(const char *[]){"Primary", "Secondary"}[devices[drive].channel], // Use the channel as an index into the array
      				(const char *[]){"Master", "Slave"}[devices[drive].drive], // Same as above, using the drive
      				devices[drive].model
			);
	}
	return err;
}

void ide_init(uint32_t BAR0, uint32_t BAR1, uint32_t BAR2, uint32_t BAR3, uint32_t BAR4) {
	size_t	i, j, k, count = 0;

	/* 1- Detect I/O Ports which interface IDE Controller */
	// Clear 2 last bits to align adresses
	// If BARx == 0, fall back on default values
	chnls[ATA_PRIMARY].base   = (BAR0 & CLEAR_TWO_LAST_BITS) + ATA_DEFAULT_BASE_PRIMARY   * (!BAR0);
	chnls[ATA_PRIMARY].crtl   = (BAR1 & CLEAR_TWO_LAST_BITS) + ATA_DEFAULT_CRTL_PRIMARY   * (!BAR1);
	chnls[ATA_SECONDARY].base = (BAR2 & CLEAR_TWO_LAST_BITS) + ATA_DEFAULT_BASE_SECONDARY * (!BAR2);
	chnls[ATA_SECONDARY].crtl = (BAR3 & CLEAR_TWO_LAST_BITS) + ATA_DEFAULT_CRTL_SECONDARY * (!BAR3);
	chnls[ATA_PRIMARY].bmide  = (BAR4 & CLEAR_TWO_LAST_BITS);
	chnls[ATA_SECONDARY].bmide= (BAR4 & CLEAR_TWO_LAST_BITS) + 8;

	/* 2- Disable IRQs */
	ide_write(ATA_PRIMARY  , ATA_REG_CONTROL, 2);
	ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);

	/* 3- Detect ATA/ATAPI devices */
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 2; j++) {
			uint8_t err = 0, type = IDE_ATA, status;
			devices[count].reserved = 0;
			// (I) Select Drive
			ide_write(i, ATA_REG_HDDEVSEL, MASTER_DRIVE | (j << 4)); // If j == 0, does nothing, if j == 1, sets bit for SLAVE_DRIVE
			sleep(10);

			// (II) Send ATA identify cmd
			ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
			sleep(10);

			// (III) Polling
			if (!ide_read(i, ATA_REG_STATUS)) continue; // No device ?
			while (1) {
				status = ide_read(i, ATA_REG_STATUS);
				if (status & ATA_SR_ERR) { err = 1; break; }
				if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break;
			}

			// (IV) Probe  for ATAPI Devices
			if (!err) {
				uint8_t low_cylinder  = ide_read(i, ATA_REG_LBA1);
				uint8_t high_cylinder = ide_read(i, ATA_REG_LBA2);
				if (low_cylinder == 0x14 && high_cylinder == 0xEB)      type = IDE_ATAPI;
				else if (low_cylinder == 0x69 && high_cylinder == 0x96) type = IDE_ATAPI;
				else continue; // Unknown Type (may not be a device)
				ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
				sleep(10);
			}

			// (V) Read Identification Device Space
			ide_read_buffer(i, ATA_REG_DATA, (uint32_t)ide_buf, 128);

			// (VI) Read Device Parameters
			devices[count].reserved		= 1;
			devices[count].type		= type;
			devices[count].channel		= i;
			devices[count].drive		= j;
			devices[count].signature	= *((uint16_t*)(ide_buf + ATA_IDENT_DEVICETYPE));
			devices[count].capabilities	= *((uint16_t*)(ide_buf + ATA_IDENT_CAPABILITIES));
			devices[count].command_sets	= *((uint32_t*)(ide_buf + ATA_IDENT_COMMANDSETS));

			// (VII) Get Size
			if (devices[count].command_sets & (1 << 26)) devices[count].size = *((uint32_t*)(ide_buf + ATA_IDENT_MAX_LBA_EXT)); // 48 bits addressing
			else devices[count].size = *((uint32_t*)(ide_buf + ATA_IDENT_MAX_LBA)); // 26 bits addressing

			// (VIII) Get Model Device
			for (k = 0; k < 40; k += 2) {
				devices[count].model[k]     = ide_buf[ATA_IDENT_MODEL + k + 1];
				devices[count].model[k + 1] = ide_buf[ATA_IDENT_MODEL + k];
			}
			devices[count].model[40] = 0;
			count++;
		}

		/* 4- Print Summary */
		for (i = 0; i < 4; i++) {
			if (devices[i].reserved == 1) {
				printf("Found %s Drive %dGB - %s\n",
					(const char *[]){"ATA", "ATAPI"}[devices[i].type],
					devices[i].size,
					devices[i].model
				);
			}
		}
	}
}
