#ifndef XARRAY_H
#define XARRAY_H

#include <memory/allocators/karena.h>
#include <bitmap.h>

typedef struct xarray_t xarray_t;
struct xarray_t
{
	arena_t*	arena;
	u8**		arrays;
	u32		arrays_index;
	u32		arrays_size;
	u32		array_size;
	u32		type_size;
	u32		size;
	u32		capacity;
};

/* Name: xarray_create
 * Description: creates an eXtensible array of size bytes that can extend to capacity.
 * */
static inline xarray_t*
xarray_create(u32 size, u32 capacity, u32 type_size)
{
	u32		arrays_size	= (capacity / size + size) * sizeof(void*);
	u32		xarray_size	= arrays_size + sizeof(xarray_t);
	arena_t*	arena		= KARENA_ALLOC(	.flags		= KARENA_FLAG_NO_CHAIN,
							.reserve_size	= capacity + xarray_size,
							.commit_size	= size + xarray_size);

	if (!arena)
		return 0;

	xarray_t	xarray = KARENA_PUSH_STRUCT(arena, xarray_t);

	xarray->arena			= arena;
	xarray->size			= size;
	xarray->array_size		= size;
	xarray->type_size		= type_size;
	xarray->capacity		= capacity;
	xarray->arrays_size		= arrays_size;
	xarray->arrays_index		= 0;
	xarray->arrays			= KARENA_PUSH_ARRAY(arena, void*, arrays_size);
	xarray->arrays[0]		= KARENA_PUSH_ARRAY(arena, u8, size);

	memset(xarray->arrays, 0, arrays_size);

	return xarray;
}

#define XARRAY_CREATE(size, type) xarray_create((size) * sizeof(type), (size) * 10, sizeof(type))

/* Name: xarray_read_at
 * Description: returns a pointer to element at index of xarray.
 * On error, returns 0.
 * */
static inline void*
xarray_read_ref_at(xarray_t* xarray, u32 index)
{
	if (index * xarray->type_size > xarray->size - xarray->type_size)
		return 0;

	u32	array_id= index / xarray->arrays_size;
	u8*	array	= xarray->arrays[array_id];
	
	return (void*)&array[(index * xarray->type_size) % xarray->arrays_size];
}

// Note: UNSAFE, can dereference a null pointer if index is out of bound.
#define XARRAY_READ_AT(arr, i, T) *(T*)xarray_read_ref_at(arr, i)

/* Name: xarray_write_at
 * Description: writes element at index of xarray.
 * On success, returns 1.
 * On error, returns 0.
 * */
static inline bool
xarray_write_at(xarray_t* xarray, u32 index, void* element)
{
	arena_t*	arena		= xarray->arena;
	void**		arrays		= xarray->arrays;
	u32		arrays_size	= xarray->arrays_size;
	u32		index_byte	= index * xarray->type_size;

	if (index_byte > xarray->capacity - index_byte)
		return 0;

	while (index_byte > xarray->size - index_byte)
	{
		arrays[xarray->arrays_index] = KARENA_PUSH_ARRAY(arena, u8, arrays_size);

		xarray->arrays_index++;
		xarray->size += arrays_size;
	}

	void*	dst = xarray_read_at(xarray, index);
	
	memcpy(dst, element, xarray->type_size);

	return 1;
}

#endif
