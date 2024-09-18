#include "ide.h"

channels 	chnls   = {0};
ide_devices 	devices = {0};
uint8_t ide_buf[2048] = {0};
volatile static uint8_t ide_irq_invoked = 0;
static uint8_t atapi_packet[12] = {ATAPI_CMD_READ, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t ide_read(uint8_t channel, uint8_t reg) {
	uint8_t result;

	if (reg > 0x07 && reg < 0x0C) // If secondary channel register, notice it's busy
		ide_write(channel, ATA_REG_CONTROL, ATA_SR_BSY | chnls[channel].nIEN);
	if (reg < 0x08)
		result = port_byte_in(chnls[channel].base + reg);
	else if (reg < 0x0C)
		result = port_byte_in(chnls[channel].base + reg - 0x06);
	else if (reg < 0x0E)
		result = port_byte_in(chnls[channel].crtl + reg - 0x0A);
	else if (reg < 0x16)
		result = port_byte_in(chnls[channel].bmide + reg - 0x0E);
	if (reg > 0x07 && reg < 0x0C) // If secondary channel register, not busy anymore
		ide_write(channel, ATA_REG_CONTROL, chnls[channel].nIEN);

	return result;
}

void    ide_write(uint8_t channel, uint8_t reg, uint8_t data) {

	if (reg > 0x07 && reg < 0x0C) // If secondary channel register, notice it's busy
		ide_write(channel, ATA_REG_CONTROL, ATA_SR_BSY | chnls[channel].nIEN);
	if (reg < 0x08)
		 port_byte_out(chnls[channel].base + reg, data);
	else if (reg < 0x0C)
		 port_byte_out(chnls[channel].base + reg - 0x06, data);
	else if (reg < 0x0E)
		 port_byte_out(chnls[channel].crtl + reg - 0x0A, data);
	else if (reg < 0x16)
		 port_byte_out(chnls[channel].bmide + reg - 0x0E, data);
	if (reg > 0x07 && reg < 0x0C) // If secondary channel register, not busy anymore
		ide_write(channel, ATA_REG_CONTROL, chnls[channel].nIEN);
}

void    ide_read_buffer(uint8_t channel, uint8_t reg, uint32_t buffer, uint32_t quads) {
	if (reg > 0x07 && reg < 0x0C) // If secondary channel register, notice it's busy
		ide_write(channel, ATA_REG_CONTROL, ATA_SR_BSY | chnls[channel].nIEN);
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
	if (reg > 0x07 && reg < 0x0C) // If secondary channel register, not busy anymore
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
      				devices[drive].model);
	}
	return err;
}

void ide_init(uint32_t BAR0, uint32_t BAR1, uint32_t BAR2, uint32_t BAR3, uint32_t BAR4) {
	//TODO: https://wiki.osdev.org/PCI_IDE_Controller#Read/Write_From_ATA_Drive
}
