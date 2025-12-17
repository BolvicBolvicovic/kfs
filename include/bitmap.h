#ifndef BITMAP_H
#define BITMAP_H

#include <c_types.h>
#include <compiler.h>

#define BITMAP_CHUNK_FULL	0xffffffff
#define BITMAP_CHUNK_SIZE	sizeof(u32)
#define BITMAP_CHUNK_LENGHT	32

/* Name: bitmap_t
 * Description: bitmap struct with BITMAP_CHUNK_SIZE of 32 bits.
 * */
typedef struct
{
	u32*	map;
	u32	size;
	u32	position;
} bitmap_t;

/* Name: bitmap_set_bit
 * Description: sets a bit in the bitmap.
 * */
static __always_inline void
bitmap_set_bit(bitmap_t* map, u32 bit)
{
	map->map[bit / 32] |= (1 << (bit % 32));
}

/* Name: bitmap_unset_bit
 * Description: unsets a bit in the bitmap.
 * */
static __always_inline void
bitmap_unset_bit(bitmap_t* map, u32 bit)
{
	map->map[bit / 32] &= ~(1 << (bit % 32));
}

/* Name: bitmap_test_bit
 * Description: returns the value of a bit in the bitmap (1 or 0).
 * */
static __always_inline u32
bitmap_test_bit(bitmap_t* map, u32 bit)
{
	return map->map[bit / 32] & (1 << (bit % 32));
} 

/* Name: bitmap_find_next_free
 * Description: finds the next free bit in the bitmap.
 * On error, returns -1.
 * */
static inline s32
bitmap_find_next_free_bit(bitmap_t* bitmap)
{
	u32	i = bitmap->position;

	do
	{
		if (i * BITMAP_CHUNK_SIZE >= bitmap->size) i = 0;

		if (i * BITMAP_CHUNK_SIZE >= bitmap->size) continue;
		if (bitmap->map[i] == BITMAP_CHUNK_FULL) continue;

		for (u32 j = 0; j < BITMAP_CHUNK_SIZE; j++)
		{
			u32	bit = 1 << j;

			if (!(bitmap->map[i] & bit))
			{
				if (j + 1 < BITMAP_CHUNK_SIZE)
					bitmap->position = i;
				else if ((i + 1) * BITMAP_CHUNK_SIZE < bitmap->size)
					bitmap->position = i + 1;
				else
					bitmap->position = 0;

				return i * BITMAP_CHUNK_SIZE + j;
			}
		}
	} while (++i != bitmap->position);

	return -1;
}

static inline s32
bitmap_find_next_free_chunk(bitmap_t* bitmap)
{
	u32	i = bitmap->position;

	do
	{
		if (i * BITMAP_CHUNK_SIZE >= bitmap->size) i = 0;

		if (bitmap->map[i]) continue;

		bitmap->position = i + 1;

		return i * BITMAP_CHUNK_SIZE;
	} while (++i != bitmap->position);

	return -1;
}

#endif
