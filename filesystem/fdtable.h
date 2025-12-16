#ifndef FDTABLES_H
#define FDTABLES_H

#include <c_types.h>
#include <atomic.h>
#include <processes/locks/spinlock.h>
#include <bitmap.h>
#include "fs.h"

#define FILE_DESCRIPTORS_ARRAY_SIZE	(5 * BITMAP_CHUNK_SIZE)

typedef struct
{
	u32		max_fds;
	file_t*		fds[FILE_DESCRIPTORS_ARRAY_SIZE];
	bitmap_t	close_on_exec;	// maps fds that should be closed when using execve
	bitmap_t	open_fds;	// maps open fds
	//bitmap_t	full_fds;	// maps chunck of fds that are full
	// TODO: look up __rcu and if it should be applied here instead of a lock
	// TODO: maybe use an arena to allocate and grow fds
	spinlock_t	fds_lock;
} fdtable_t;

/* Name: fd_get
 * Description: gets the next free file descriptor available.
 * On error, returns -1.
 * */
s32	fd_get(bool close_on_exec);

/* Name: fd_install
 * Description: install a file descriptor in the table.
 * On error, returns -1.
 * */
s32	fd_install(s32 fd, file_t* file);

/* Name: fd_free
 * Description: marks file descriptor as available.
 * */
void	free_fd(s32 fd);

#endif
