#ifndef BITMAP_H
#define BITMAP_H

#include <c_types.h>
#include <compiler.h>

// TODO: document

typedef struct
{
	u32*	map;
	u32	size;
} bitmap_t;

static __always_inline void
bitmap_set_bit(bitmap_t* map, u32 bit)
{
	map->map[bit / 32] |= (1 << (bit % 32));
}

static __always_inline void
bitmap_unset_bit(bitmap_t* map, u32 bit)
{
	map->map[bit / 32] &= ~(1 << (bit % 32));
}

static __always_inline u32
bitmap_test_bit(bitmap_t* map, u32 bit)
{
	return map->map[bit / 32] & (1 << (bit % 32));
} 

#define BITMAP_CHUNK_FULL	0xffffffff
#define BITMAP_CHUNK_SIZE	sizeof(u32)

#endif
