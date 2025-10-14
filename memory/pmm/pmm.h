#ifndef PMM_H
#define PMM_H

#include "../../lib/stdlib/stdlib.h"
#include "../../lib/string/string.h"
#include "../../lib/stdio/stdio.h"
#include <stdint.h>
#include <stddef.h>

#define PMM_BLOCKS_PER_BYTE 8
#define PMM_BLOCK_SIZE 0x1000
#define PMM_BLOCK_ALIGN PMM_BLOCK_SIZE

void		pmm_init(size_t mem_size, uint32_t bitmap);
void		pmm_init_region(uint32_t base, size_t size);
void		pmm_deinit_region(uint32_t base, size_t size);
uint32_t	pmm_alloc_block();
uint32_t	pmm_alloc_blocks(size_t nb_blocks);
void		pmm_free_block(uint32_t p);
void		pmm_free_blocks (uint32_t p, size_t size);

#endif
