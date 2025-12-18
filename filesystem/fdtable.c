#include "fdtable.h"

static fdtable_t	fd_table=
{
	.max_fds	= FILE_DESCRIPTORS_ARRAY_SIZE,
	.fds		= 0,
	.close_on_exec	= { 0, FILE_DESCRIPTORS_ARRAY_SIZE, 0 },
	.open_fds	= { 0, FILE_DESCRIPTORS_ARRAY_SIZE, 0 },
	.fds_lock	= { 0 },
};

s32
fd_get(bool close_on_exec)
{
	bitmap_t*	fds = &fd_table.open_fds;
	bitmap_t*	coe = &fd_table.close_on_exec;
	spinlock_t*	lock= &fd_table.fds_lock;

	spinlock_lock(lock);
	
	u32	bit = bitmap_find_next_free_bit(fds);

	if (bit == -1)
	{
		spinlock_unlock(lock);
		return -1;
	}
	
	bitmap_set_bit(fds, bit);

	if (close_on_exec)
		bitmap_set_bit(coe, bit);

	spinlock_unlock(lock);

	return bit;
}

s32
fd_install(s32 fd, file_t* file)
{
	fd_table.fds->lock[fd]		= file->lock;
	fd_table.fds->mode[fd]		= file->mode;
	fd_table.fds->flags[fd]		= file->flags;
	fd_table.fds->inode[fd]		= file->inode;
	fd_table.fds->private_data[fd]	= file->private_data;
	fd_table.fds->operations[fd]	= file->operations;
	fd_table.fds->mapping[fd]	= file->mapping;
	fd_table.fds->path[fd]		= file->path;
	fd_table.fds->owner[fd]		= file->owner;
	fd_table.fds->credentials[fd]	= file->credentials;
	fd_table.fds->position[fd]	= file->position;
	fd_table.fds->arena[fd]		= file->arena;

	return 0;
}

void
fd_free(s32 fd)
{
	bitmap_t*	fds = &fd_table.open_fds;
	bitmap_t*	coe = &fd_table.close_on_exec;

	bitmap_unset_bit(fds, fd);
	bitmap_unset_bit(coe, fd);
}

void
fd_init(karena_t* arena)
{
	fd_table.fds = file_allocate_soa(arena, FILE_DESCRIPTORS_ARRAY_SIZE);
}
