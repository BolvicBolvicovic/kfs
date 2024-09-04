#ifndef PMM_H
#define PMM_H

#include "../../lib/stdlib/stdlib.h"
#include "../../lib/string/string.h"
#include <stdint.h>
#include <stddef.h>

#define PMM_BLOCKS_PER_BYTE 8
#define PMM_BLOCK_SIZE 0x1000
#define PMM_BLOCK_ALIGN PMM_BLOCK_SIZE

void pmm_init(size_t mem_size, uint32_t bitmap);
void pmm_init_region(uint32_t base, size_t size);
void pmm_deinit_region(uint32_t base, size_t size);
void* pmm_alloc_block();
void* pmm_alloc_blocks(size_t nb_blocks);
void pmm_free_block(void* p);

#endif
