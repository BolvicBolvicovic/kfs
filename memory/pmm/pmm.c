#include "pmm.h"
#include <processes/locks/spinlock.h>
#include <bitmap.h>

static u32	_memory_used_blocks	= 0;
static u32	_memory_max_blocks	= 0;
static bitmap_t	_memory_map		= { 0, 0, 0 };

// TODO: look if mutex would be better than spinlock here
static SPINLOCK_DEFINE(sl_pmm);

static u32
mmap_find_first_free_s(u32 size)
{
	if (size == 0) return (u32)-1;
	if (size == 1) return bitmap_find_next_free_bit(&_memory_map);
	if (size <= BITMAP_CHUNK_SIZE && size > BITMAP_CHUNK_SIZE / 2)
		return bitmap_find_next_free_chunk(&_memory_map);

	for (u32 i = 0; i < _memory_map.size; i++)
	{
		if (_memory_map.map[i] == BITMAP_CHUNK_FULL) continue;

		for (u32 j = 0; j < BITMAP_CHUNK_SIZE; j++)
		{
			// Note test each bit in the dword
			u32	bit = 1<<j;

			if (_memory_map.map[i] & bit) continue;

			u32	starting_bit = i * BITMAP_CHUNK_SIZE + j;
 			// Note: loop through each bit to see if its enough space
			u32	free = 0;

			for (u32 count = 0; count < size; count++)
			{
				// Note: this bit is clear (free frame)
				if (!bitmap_test_bit(&_memory_map, starting_bit + count))
					free++;
				// Note: free count==size needed; return index
				if (free==size)
				{
					_memory_map.position = starting_bit + size + 1;
					return starting_bit;
				}
			}
		}
	}

	return (u32)-1;
}

void
pmm_init(u32 mem_size, u32 bitmap)
{
	_memory_max_blocks	= mem_size * 1024 / PMM_BLOCK_SIZE;
	_memory_map.size	= _memory_max_blocks / PMM_BLOCKS_PER_BYTE / 4;
	_memory_map.map		= (u32*)bitmap;
	
	memset(_memory_map.map, 0, _memory_map.size * 4);
}

void
pmm_init_region(u32 base, u32 size)
{
	s32	align	= base / PMM_BLOCK_SIZE;
	s32	blocks	= size / PMM_BLOCK_SIZE;

	spinlock_lock(&sl_pmm);

	for (; blocks > 0; blocks--)
	{
		bitmap_unset_bit(&_memory_map, align++);
		_memory_used_blocks--;
	}

	spinlock_unlock(&sl_pmm);
}

void
pmm_deinit_region(u32 base, u32 size)
{
	s32	align	= base / PMM_BLOCK_SIZE;
	s32	blocks	= size / PMM_BLOCK_SIZE;

	spinlock_lock(&sl_pmm);

	for (; blocks > 0; blocks--)
	{
		bitmap_set_bit(&_memory_map, align++);
		_memory_used_blocks++;
	}

	spinlock_unlock(&sl_pmm);
}

u32
pmm_alloc_block()
{
	spinlock_lock(&sl_pmm);

	if (_memory_max_blocks - _memory_used_blocks <= 0)
	{
		spinlock_unlock(&sl_pmm);
		return 0;
	}
	
	s32	frame = bitmap_find_next_free_bit(&_memory_map);

	if (frame == -1)
	{
		spinlock_unlock(&sl_pmm);
		return 0;
	}
	
	bitmap_set_bit(&_memory_map, frame);
	_memory_used_blocks++;

	spinlock_unlock(&sl_pmm);
	
	return (u32)(frame * PMM_BLOCK_SIZE);
}

u32
pmm_alloc_blocks(u32 nb_blocks)
{
	spinlock_lock(&sl_pmm);

	if (_memory_max_blocks - _memory_used_blocks <= 0)
	{
		spinlock_unlock(&sl_pmm);
		return 0;
	}
	
	s32	frame = mmap_find_first_free_s(nb_blocks);

	if (frame == -1)
	{
		spinlock_unlock(&sl_pmm);
		return 0;
	}
	
	for (u32 i = 0; i < nb_blocks; i++)
	{
		bitmap_set_bit(&_memory_map, frame + i);
	}

	_memory_used_blocks += nb_blocks;

	spinlock_unlock(&sl_pmm);
	
	return (u32)(frame * PMM_BLOCK_SIZE);
}

void
pmm_free_block(u32 p)
{
	s32	frame = p / PMM_BLOCK_SIZE;

	spinlock_lock(&sl_pmm);

	bitmap_unset_bit(&_memory_map, frame);
	_memory_used_blocks--;

	spinlock_unlock(&sl_pmm);
}

void
pmm_free_blocks(u32 p, u32 size)
{
	s32	frame = p / PMM_BLOCK_SIZE;

	spinlock_lock(&sl_pmm);

	for (u32 i = 0; i < size; i++)
	{
		bitmap_unset_bit(&_memory_map, frame + i);
	}

	_memory_used_blocks -= size;

	spinlock_unlock(&sl_pmm);
}
