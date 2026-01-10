#ifndef PATH_H
#define PATH_H

#include <c_types.h>
#include <memory/allocators/karena.h>

#define PATH_MAX_SIZE			0x1000
#define PATH_MAX_NODES			(PATH_MAX_SIZE / 2)
#define DIRECTORY_ENTRY_MAX_CHILDREN	100
#define DIRECTORY_ENTRY_MAX_NAME_SIZE	32

struct file_t;

enum
{
	PATH_ERROR_EXCEED_MAX_SIZE	= -1,
	PATH_ERROR_NO_ROOT		= -2,
	PATH_ERROR_NO_FILE		= -3,
	PATH_ERROR_NO_ENTRY_LEFT	= -4,
};

typedef struct directory_entry_t directory_entry_t;
struct directory_entry_t
{
	char			name[DIRECTORY_ENTRY_MAX_NAME_SIZE];
	// TODO: create the hashmap that maps a dir to a inode
	struct file_t*		inode;
	directory_entry_t*	parent;
	directory_entry_t*	children[DIRECTORY_ENTRY_MAX_CHILDREN];
	directory_entry_t**	sibilings;
};

typedef struct path_t path_t;
struct path_t
{
	directory_entry_t*	directory_entry;
	directory_entry_t*	mounted_root;
};

s32	path_bind(karena_t* arena, path_t* path, char* addr, u32 addr_len);

#endif
