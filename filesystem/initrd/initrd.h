#ifndef INITRD_H
#define INITRD_H

#include "../vfs/vfs.h"

typedef struct {
	uint32_t nfiles;
} initrd_header_t;

typedef struct {
	uint8_t magic;
	char 	name[64];
	uint32_t offset;
	uint32_t len;
} initrd_file_header_t;

fs_node_t *initialise_initrd(uint32_t location);

#endif
