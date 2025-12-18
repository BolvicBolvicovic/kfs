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
	.path	= { .directory_entry = &root_entry, .mounted_root = &root_entry },
};

void
fs_init(void)
{
	s32	root_fd	= fd_get(0);

	root_folder.arena = KARENA_ALLOC();
	fd_install(root_fd, &root_folder);
}

file_t*
fs_get_root(void)
{
	return &root_folder;
}

inline soa_file_t*
file_allocate_soa(karena_t* arena, u32 size)
{
	soa_file_t*	soa = 0;
#define SOA		soa
#define SOA_ARENA	arena
#define SOA_SIZE	size
	SOA_ALLOC_KARENA_STRUCT_OF_ARRAYS(soa_file_t, FILE_FIELDS);
#undef SOA
#undef SOA_ARENA
#undef SOA_SIZE
	return soa;
}

file_t*
fs_get_file(char* addr, u32 addr_len)
{
	// TODO: explore the idea of using index node to connect an address to an index
	// TODO: maybe return an ID instead of a pointer
	(void)addr;
	(void)addr_len;
	return 0;
}
