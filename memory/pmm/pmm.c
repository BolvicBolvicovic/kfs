#include "pmm.h"
#include <processes/locks.h>

static u32	_memory_size		= 0;
static u32	_memory_used_blocks	= 0;
static u32	_memory_max_blocks	= 0;
static u32*	_memory_map		= 0;

// TODO: look if mutex would be better than spinlock here
static SPINLOCK_DEFINE(sl_pmm);

static void
mmap_set(s32 bit)
{
	_memory_map[bit / 32] |= (1 << (bit % 32));
}

static void
mmap_unset(s32 bit)
{
	_memory_map[bit / 32] &= ~(1 << (bit % 32));
}

static s32
mmap_test(s32 bit)
{
	return _memory_map[bit / 32] & (1 << (bit % 32));
} 

static s32
mmap_find_first_free()
{
	for (u32 i = 0; i < _memory_size; i++)
	{
		if (_memory_map[i] != 0xFFFFFFFF)
		{
			for (u32 j = 0; j < 32; j++)
	    		{
				u32 bit = 1 << j;
				if (!(_memory_map[i] & bit)) return i * 32 + j;
			}
		}
	}

	return (-1);
}

static s32
mmap_find_first_free_s (u32 size)
{
	if (size==0) return -1;
	if (size==1) return mmap_find_first_free();

	for (u32 i = 0; i < _memory_size; i++)
	{
		if (_memory_map[i] != 0xffffffff)
		{
			for (u32 j=0; j<32; j++)
			{
				// Note test each bit in the dword
				u32 bit = 1<<j;
				if (!(_memory_map[i] & bit))
				{
					u32 startingBit = i*32 + j;
					u32 free=0; // Note: loop through each bit to see if its enough space

					for (u32 count=0; count<size;count++)
					{
						if (! mmap_test(startingBit+count)) free++;	// Note: this bit is clear (free frame)
						if (free==size) return startingBit; 			// Note: free count==size needed; return index
					}
				}
			}
		}
	}

	return -1;
}

void
pmm_init(u32 mem_size, u32 bitmap)
{
	_memory_map		= (u32*)bitmap;
	_memory_max_blocks	= mem_size * 1024 / PMM_BLOCK_SIZE;
	_memory_size		= _memory_max_blocks / PMM_BLOCKS_PER_BYTE / 4;
	
	memset(_memory_map, 0, _memory_size * 4);
}

void
pmm_init_region(u32 base, u32 size)
{
	s32	align	= base / PMM_BLOCK_SIZE;
	s32	blocks	= size / PMM_BLOCK_SIZE;

	spinlock_lock(&sl_pmm);

	for (; blocks > 0; blocks--)
	{
		mmap_unset(align++);
		_memory_used_blocks--;
	}

	spinlock_unlock(&sl_pmm);

	mmap_set(0);
}

void
pmm_deinit_region(u32 base, u32 size)
{
	s32	align	= base / PMM_BLOCK_SIZE;
	s32	blocks	= size / PMM_BLOCK_SIZE;

	spinlock_lock(&sl_pmm);

	for (; blocks > 0; blocks--)
	{
		mmap_set(align++);
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
	
	s32	frame = mmap_find_first_free();

	if (frame == -1)
	{
		spinlock_unlock(&sl_pmm);
		return 0;
	}
	
	mmap_set(frame);
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
		mmap_set(frame + i);
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

	mmap_unset(frame);
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
		mmap_unset(frame + i);
	}

	_memory_used_blocks -= size;

	spinlock_unlock(&sl_pmm);
}
