#include "path.h"
#include <slice.h>
#include <string.h>

s32
path_bind(karena_t* arena, path_t* path, char* addr, u32 addr_len)
{
	if (addr_len > PATH_MAX_SIZE)
		return PATH_ERROR_EXCEED_MAX_SIZE;
	// TODO: handle non absolute path cases
	if (addr[0] != '/')
		return PATH_ERROR_NO_ROOT;

	slice_t			slice	= { 0, 0 };
	directory_entry_t*	cwd	= path->mounted_root;
	u32			i	= 0;
	u32			counting= 0;

	for (; i < addr_len; i++)
	{
		if (counting && addr[i] == '/')
		{
			slice.end	= i;
			counting	= 0;
			
			u32	j	= 0;
			char*	name 	= addr + slice.start;
			u32	name_len= slice.end - slice.start;
			
			if (name_len > DIRECTORY_ENTRY_MAX_NAME_SIZE)
				return PATH_ERROR_EXCEED_MAX_SIZE;

			for (; j < DIRECTORY_ENTRY_MAX_CHILDREN; j++)
			{
				if (memcmp(name, cwd->children[i]->name, name_len) != 0)
					continue;

				cwd = &cwd[i];
				break;
			}
			
			if (j == DIRECTORY_ENTRY_MAX_CHILDREN)
				return PATH_ERROR_NO_FILE;
		}
		else if (!counting && !(addr[i] == '/'))
		{
			counting	= 1;
			slice.start	= i;
		}
	}
	
	if (counting)
		slice.end = i;

	char*	ne_name		= addr + slice.start;
	u32	ne_name_len	= slice.end - slice.start;

	if (ne_name_len > DIRECTORY_ENTRY_MAX_NAME_SIZE)
		return PATH_ERROR_EXCEED_MAX_SIZE;

	u32	j = 0;

	for (; j < DIRECTORY_ENTRY_MAX_CHILDREN && cwd->children[j]; j++);

	if (j == DIRECTORY_ENTRY_MAX_NAME_SIZE)
		return PATH_ERROR_NO_ENTRY_LEFT;

	directory_entry_t*	new_entry  = KARENA_PUSH_STRUCT(arena, directory_entry_t);
	// TODO: maybe add a lock in path when creating a new entry
	cwd->children[j]	= new_entry;
	path->directory_entry	= new_entry;

	memcpy(new_entry->name, ne_name, ne_name_len);
	new_entry->parent	= cwd;
	new_entry->sibilings	= cwd->children;

	return 0;
}
