#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>
#include "../../lib/string/string.h"
#include "../../memory/vmm/vmm.h"

enum node_type {
	FS_FILE,
	FS_DIR,
	FS_CHARDEVICE,
	FS_BLOCKDEVICE,
	FS_PIPE,
	FS_SYMLINK,
	FS_MOUNTPOINT
};

enum open_flags {
	READ,
	WRITE
};

typedef	struct _dentry {
	char		name[32];
	uint32_t	inode;
} dentry;

typedef struct _FILE {
	char		name[32];
	uint32_t	permissions;
	enum node_type	type;
	uint32_t	uid;
	uint32_t	sid;
	uint32_t	inode;
	uint32_t	length;
	struct node_operations {
		uint32_t	(*read)(struct _FILE*,uint32_t,uint32_t,char*buffer);
		uint32_t	(*write)(struct _FILE*,uint32_t,uint32_t,char*buffer);
		uint32_t	(*open)(struct _FILE*, enum open_flags);
		uint32_t	(*close)(struct _FILE*);
		dentry*		(*readdir)(struct _FILE*,uint32_t);
	} node_operations;
} FILE, *PFILE;

uint32_t	fs_close(PFILE);
uint32_t	fs_write(PFILE,uint32_t,uint32_t,char*buffer);
uint32_t	fs_read(PFILE,uint32_t,uint32_t,char*buffer);
uint32_t	fs_open(PFILE, enum open_flags);
dentry*		fs_readdir(PFILE,uint32_t);

#endif
