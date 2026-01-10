#include "fs.h"
#include "fdtable.h"
#include <interrupt_macros.h>

static directory_entry_t	root_entry =
{
	.name		= "__root__",
	.inode		= 0,
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
	
	if (fd_install(root_fd, &root_folder) == -1)
		INT_DEBUG();

	root_entry.inode = &root_folder;
}

file_t*
fs_get_root(void)
{
	return &root_folder;
}

file_t*
fs_get_file_from_addr(char* addr, u32 addr_len)
{
	// TODO: explore the idea of using index node to connect an address to an index
	// TODO: maybe return an ID instead of a pointer
	// TODO: handle current working directory
	
	file_t* result = &root_folder;
	
	for (u32 i = 0; i < addr_len; i++)
	{
		if (addr[i] == '/')
			continue;
			
		u32			j = 0;
		directory_entry_t**	children = result->path.directory_entry->children;
		
		for (u32 k = 0; k < DIRECTORY_ENTRY_MAX_CHILDREN; k++)
		{
			if (!children[k])
				continue;

			for (	j = 0;
				i + j < addr_len && addr[i + j] != '/' && children[k]->name[j];
				j++);

			if (!children[k]->name[j] && addr[i + j == '/'])
			{
				result = children[k]->inode;
				break;
			}
		}

		i += j;
	}

	return result;
}

s32
fs_read(s32 fd, char* buf, u32 buf_size)
{
	file_t*	file= fd_get_file(fd);

	if (!file)
		return -1;
	// TODO: check if I should add pos instead of 0
	return file->operations.read(file, buf, buf_size, 0);
}

s32
fs_write(s32 fd, char* buf, u32 buf_size)
{
	file_t*	file= fd_get_file(fd);

	if (!file)
		return -1;
	
	// TODO: check if I should add pos instead of 0
	return file->operations.write(file, buf, buf_size, 0);
}

s32
fs_close(s32 fd)
{
	file_t*	file= fd_get_file(fd);

	if (!file)
		return -1;
	
	return file->operations.close(file);
}
