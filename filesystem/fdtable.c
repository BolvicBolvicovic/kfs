#include "fdtable.h"

static u32		coe_map[FILE_DESCRIPTORS_CHUNKS]	= { 0 };
static u32		ofds_map[FILE_DESCRIPTORS_CHUNKS]	= { 0 };
static fdtable_t	fd_table =
{
	.max_fds	= FILE_DESCRIPTORS_ARRAY_SIZE,
	.fds		= { 0 },
	.close_on_exec	= { coe_map, FILE_DESCRIPTORS_ARRAY_SIZE, 0 },
	.open_fds	= { ofds_map, FILE_DESCRIPTORS_ARRAY_SIZE, 0 },
	.fds_lock	= { 0 },
};

s32
fd_get(bool close_on_exec)
{
	bitmap_t*	ofds= &fd_table.open_fds;
	bitmap_t*	coe = &fd_table.close_on_exec;
	spinlock_t*	lock= &fd_table.fds_lock;

	spinlock_lock(lock);
	
	u32	bit = bitmap_find_next_free_bit(ofds);

	if (bit == -1)
	{
		spinlock_unlock(lock);
		return -1;
	}
	
	bitmap_set_bit(ofds, bit);

	if (close_on_exec)
		bitmap_set_bit(coe, bit);

	spinlock_unlock(lock);

	return bit;
}

s32
fd_install(s32 fd, file_t* file)
{
	bitmap_t*	ofds= &fd_table.open_fds;
	bitmap_t*	coe = &fd_table.close_on_exec;
	spinlock_t*	lock= &fd_table.fds_lock;
	file_t**	fds = fd_table.fds;

	spinlock_lock(lock);

	// Note: fd must be set previously with fd_get
	if (!bitmap_test_bit(ofds, fd))
	{
		spinlock_unlock(lock);
		return -1;
	}

	if (file->flags & FILE_MODE_NO_REUSE)
		bitmap_set_bit(coe, fd);

	bitmap_set_bit(ofds, fd);
	fds[fd] = file;

	spinlock_unlock(lock);

	return 0;
}

void
fd_free(s32 fd)
{
	bitmap_t*	ofds= &fd_table.open_fds;
	bitmap_t*	coe = &fd_table.close_on_exec;

	bitmap_unset_bit(ofds, fd);
	bitmap_unset_bit(coe, fd);
}

file_t*
fd_get_file(s32 fd)
{
	spinlock_lock(&fd_table.fds_lock);

	if (!bitmap_test_bit(&fd_table.open_fds, fd))
	{
		spinlock_unlock(&fd_table.fds_lock);
		return 0;
	}

	file_t*	file = fd_table.fds[fd];

	spinlock_unlock(&fd_table.fds_lock);

	return file;
}
