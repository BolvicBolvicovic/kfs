#include "fs.h"
#include "fdtable.h"

static directory_entry_t	root_entry =
{
	.name		= "__root__",
	.parent 	= 0,
	.sibilings	= 0,
	.children	= { 0 }
};

static file_t			root_folder=
{
	.path	= { .directory_entry = &root_entry, .root_entry = &root_entry },
	.arena	= KARENA_ALLOC();
};

void
fs_init(void)
{
	s32	root_fd = fd_get();

	fd_install(root_fd, &root_folder);
}

file_t*
fs_get_root(void)
{
	return &root_folder;
}
