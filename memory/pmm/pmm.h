#ifndef PMM_H
#define PMM_H

#include <lib/stdlib/stdlib.h>
#include <lib/string/string.h>
#include <lib/stdio/stdio.h>
#include <c_types.h>

#define PMM_BLOCKS_PER_BYTE 8
#define PMM_BLOCK_SIZE 0x1000
#define PMM_BLOCK_ALIGN PMM_BLOCK_SIZE

void	pmm_init(u32 mem_size, u32 bitmap);
void	pmm_init_region(u32 base, u32 size);
void	pmm_deinit_region(u32 base, u32 size);
u32	pmm_alloc_block();
u32	pmm_alloc_blocks(u32 nb_blocks);
void	pmm_free_block(u32 p);
void	pmm_free_blocks (u32 p, u32 size);

#endif
