#ifndef FDTABLES_H
#define FDTABLES_H

#include <c_types.h>
#include <atomic.h>
#include <processes/locks/spinlock.h>
#include "fs.h"

typedef struct
{
	u32		max_fds;
	file_t**	fds;
	bitmap_t	close_on_exec;	// maps fds that should be closed when using execve
	bitmap_t	open_fds;	// maps open fds
	bitmap_t	full_fds;	// maps chuck of fds that are full
	// TODO: look up __rcu and if it should be applied here instead of a lock
	// TODO: maybe use an arena to allocate and grow fds
	spinlock_t	fds_lock;
} fdtable_t;

#endif
