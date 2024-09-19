#ifndef IDE_H
#define IDE_H

/* IDE (Integrated Drive Electronics) */

#include "../../lib/stdio/stdio.h"
#include "../../drivers/pit/pit.h"

/* Status */
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

/* Errors */
#define ATA_ER_BBK      0x80    // Bad block
#define ATA_ER_UNC      0x40    // Uncorrectable data
#define ATA_ER_MC       0x20    // Media changed
#define ATA_ER_IDNF     0x10    // ID mark not found
#define ATA_ER_MCR      0x08    // Media change request
#define ATA_ER_ABRT     0x04    // Command aborted
#define ATA_ER_TK0NF    0x02    // Track 0 not found
#define ATA_ER_AMNF     0x01    // No address mark

/* Commands */
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

// These are specific commands for ATAPI
#define      ATAPI_CMD_READ       0xA8
#define      ATAPI_CMD_EJECT      0x1B

/* Indentificators to read from identification space */
/* (after ATA_CMD_IDENTIFY or ATA_CMD_IDENTIFY_PACKET write in port Status/Commands) */
#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

/* Interface type */
#define IDE_ATA        0x00
#define IDE_ATAPI      0x01

#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01

/* Task File (ports that are offsets from BAR0 and/or BAR2)*/
#define ATA_REG_DATA       0x00 // RW
#define ATA_REG_ERROR      0x01 // RO
#define ATA_REG_FEATURES   0x01 // WO
#define ATA_REG_SECCOUNT0  0x02 // RW
#define ATA_REG_LBA0       0x03 // RW (LBA == Logical Block Addressing)
#define ATA_REG_LBA1       0x04 // RW
#define ATA_REG_LBA2       0x05 // RW
#define ATA_REG_HDDEVSEL   0x06 // RW, used to select a drive in channel
#define ATA_REG_COMMAND    0x07 // WO
#define ATA_REG_STATUS     0x07 // RO
#define ATA_REG_SECCOUNT1  0x08 // 
#define ATA_REG_LBA3       0x09 // 
#define ATA_REG_LBA4       0x0A // 
#define ATA_REG_LBA5       0x0B // 
#define ATA_REG_CONTROL    0x0C // WO
#define ATA_REG_ALTSTATUS  0x0C // RO
#define ATA_REG_DEVADDRESS 0x0D // ?

// Channels:
#define      ATA_PRIMARY      0x00
#define      ATA_SECONDARY    0x01

// Directions:
#define      ATA_READ      0x00
#define      ATA_WRITE     0x01

typedef struct ide_channel_registers {
	uint16_t base; // I/O Base
	uint16_t crtl; // Control Base
	uint16_t bmide;// Bus Master IDE
	uint8_t  nIEN; // (No Interrupt)

} channels[2];

typedef struct ide_device {
	uint8_t  reserved; // 0 (Empty) or 1 (Drive really exists)
	uint8_t  channel;  // 0 (primary channel) or 1 (secondary channel)
	uint8_t  drive;    // 0 (Master Drive) or 1 (Slave Drive)
	uint16_t type;     // 0 (ATA) or 1 (ATAPI)
	uint16_t signature;// Drive signature
	uint16_t capabilities; // features
	uint32_t command_sets;
	uint32_t size;
	uint8_t  model[41];// model of the drive in string
} ide_devices[4];

/*
    BAR0: Base address of primary channel in PCI native mode (8 ports)
    BAR1: Base address of primary channel control port in PCI native mode (4 ports)
    BAR2: Base address of secondary channel in PCI native mode (8 ports)
    BAR3: Base address of secondary channel control port in PCI native mode (4 ports)
    BAR4: Bus master IDE (16 ports, 8 for each channel)
*/

uint8_t ide_read(uint8_t channel, uint8_t reg);
void    ide_write(uint8_t channel, uint8_t reg, uint8_t data);
void    ide_read_buffer(uint8_t channel, uint8_t reg, uint32_t buffer, uint32_t quads);
uint8_t ide_polling(uint8_t channel, uint32_t advanced_check);
uint8_t ide_print_error(uint32_t drive, uint8_t err);
void    ide_init(uint32_t BAR0, uint32_t BAR1, uint32_t BAR2, uint32_t BAR3, uint32_t BAR4);

#endif
