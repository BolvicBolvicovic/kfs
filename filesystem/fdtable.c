#include "fdtable.h"

static fdtable_t	fd_table=
{
	.max_fds	= FILE_DESCRIPTORS_ARRAY_SIZE,
	.fds		= { 0 },
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
	
	if (fd_table.fds == 0)

	u32		bit = bitmap_find_next_free_bit(fds);

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
	fd_table.fds[fd] = file;
}

void
fd_free(s32 fd)
{
	bitmap_t*	fds = &fd_table.open_fds;
	bitmap_t*	coe = &fd_table.close_on_exec;

	bitmap_unset_bit(fds, fd);
	bitmap_unset_bit(coe, fd);
}
