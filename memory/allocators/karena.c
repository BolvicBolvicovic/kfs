#include "karena.h"
#include <memory/vmm/vmm.h>
#include <stdlib.h>
#include <linked_list.h>

karena_t*
karena_alloc(karena_parameters_t* params)
{
	u32	reserve_size	= params->reserve_size;
	u32	commit_size	= params->commit_size;

	if (params->flags & KARENA_FLAG_LARGE_PAGES)
	{
		reserve_size	= ALIGN(reserve_size, PAGE_LARGE_SIZE);
		commit_size	= ALIGN(commit_size, PAGE_LARGE_SIZE);
	}
	else
	{
		reserve_size	= ALIGN(reserve_size, PAGE_SIZE);
		commit_size	= ALIGN(commit_size, PAGE_SIZE);
	}

	void*	base = params->optional_backbuffer;

	if (base == 0)
	{
		if (params->flags & KARENA_FLAG_LARGE_PAGES)
		{
			// TODO: handle large pages
			return 0;
		}
		else
		{
			base = vmm_reserve_kblocks(reserve_size / PAGE_SIZE);
			vmm_commit_kblocks((u32)base, commit_size / PAGE_SIZE);
		}
	}

	if (base == 0)
		return 0;

	karena_t*	arena		= (karena_t*)base;

	arena->current			= arena;
	arena->flags			= params->flags;
	arena->commit_size		= params->commit_size;
	arena->reserve_size		= params->reserve_size;
	arena->base_position		= 0;
	arena->position			= KARENA_HEADER_SIZE;
	arena->commit			= commit_size;
	arena->reserve			= reserve_size;
	arena->allocation_site_file	= params->allocation_site_file;
	arena->allocation_site_line	= params->allocation_site_line;

	return arena;
}

void
karena_release(karena_t* arena)
{
	for (karena_t *n = arena->current, *prev = 0; n; n = prev)
	{
		prev = n->prev;
		vmm_free_blocks((u32)n, n->reserve / PAGE_SIZE);
	}
}

void*
karena_push(karena_t* arena, u32 size, u32 align, bool zero)
{
	karena_t*	current = arena->current;
	u32		pos_pre = ALIGN(current->position, align);
	u32		pos_pst = pos_pre + size;

	if (current->reserve < pos_pst && !(arena->flags & KARENA_FLAG_NO_CHAIN))
	{
		u32	res_size = current->reserve_size;
		u32	cmt_size = current->commit_size;

		if (size + KARENA_HEADER_SIZE > res_size)
		{
			res_size = ALIGN(size + KARENA_HEADER_SIZE, align);
			cmt_size = ALIGN(size + KARENA_HEADER_SIZE, align);
		}

		karena_t*	new_block = KARENA_ALLOC(
				.reserve_size		= res_size,
				.commit_size		= cmt_size,
				.flags			= current->flags,
				.allocation_site_file	= current->allocation_site_file,
				.allocation_site_line	= current->allocation_site_line);

		new_block->base_position = current->base_position + current->reserve;
		SLL_STACK_PUSH_N(arena->current, new_block, prev);

		current = new_block;
		pos_pre = ALIGN(current->position, align);
		pos_pst = pos_pre + size;
	}

	u32	size_to_zero = 0;

	if (zero)
		size_to_zero = MIN(current->commit, pos_pst) - pos_pre;

	if (current->commit < pos_pst)
	{
		u32	commit_post_aligned	= pos_pst + current->commit_size - 1;
		commit_post_aligned 		-=  commit_post_aligned % current->commit_size;
		u32	commit_post_clamped	= CLAMP_TOP(commit_post_aligned, current->reserve);
		u32	commit_size		= commit_post_clamped - current->commit;
		u32	commit_ptr		= (u32)current + current->commit;

		if (current->flags & KARENA_FLAG_LARGE_PAGES)
		{
			// TODO when large page handled
		}
		else
			vmm_commit_kblocks(commit_ptr, commit_size / PAGE_SIZE);

		current->commit = commit_post_clamped;
	}

	void*	result = 0;

	if (current->commit < pos_pst)
		return 0;

	result = (u8*)current + pos_pre;
	current->position = pos_pst;
	
	if (size_to_zero)
		memset(result, 0, size_to_zero);

	return result;
}

u32
karena_pos(karena_t* arena)
{
	karena_t*	current	= arena->current;
	u32		pos	= current->base_position + current->position;

	return pos;
}

void
karena_pop_to(karena_t* arena, u32 pos)
{
	u32		big_pos = CLAMP_BOT(KARENA_HEADER_SIZE, pos);
	karena_t*	current = arena->current;

	for (karena_t* prev = 0; current->base_position >= big_pos; current = prev)
	{
		prev = current->prev;
		vmm_free_blocks((u32)current, current->reserve / PAGE_SIZE);
	}

	u32	new_pos = big_pos - current->base_position;

	if (new_pos > current->position)
		return;
	
	arena->current		= current;
	current->position	= new_pos;
}

inline void
karena_clear(karena_t* arena)
{
	karena_pop_to(arena, 0);
}

void
karena_pop(karena_t* arena, u32 amount)
{
	u32	pos_old = karena_pos(arena);
	u32	pos_new = pos_old;

	if (amount < pos_old)
		pos_new = pos_old - amount;

	karena_pop_to(arena, pos_new);
}

inline karena_temp_t
karena_temp_begin(karena_t* arena)
{
	u32		pos = karena_pos(arena);
	karena_temp_t	temp= { arena, pos };

	return temp;
}

inline void
karena_temp_end(karena_temp_t temp)
{
	karena_pop_to(temp.arena, temp.pos);
}
