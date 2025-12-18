#ifndef FS_H
#define FS_H

#include <c_types.h>
#include <linked_list.h>
#include <compiler.h>
#include <bits.h>
#include <atomic.h>
#include <soa.h>
#include <processes/locks/spinlock.h>
#include <memory/allocators/karena.h>
#include "path.h"

struct file_t;
struct mm_t;

#define FILE_TYPE_REGULAR		0100000
#define FILE_TYPE_DIRECTORY		0040000
#define FILE_TYPE_SYMLINK		0120000
#define FILE_TYPE_SOCKET		0140000
#define FILE_TYPE_BLOCK_DEVICE		0060000
#define FILE_TYPE_CHAR_DEVICE		0020000
#define FILE_TYPE_FIFO			0010000

#define FILE_PERMISSION_RWXU		0000700
#define FILE_PERMISSION_RWXG		0000070
#define FILE_PERMISSION_RWXO		0000007

typedef struct inode_t inode_t;
struct inode_t
{
	// Note: File type & permissions
	u32	mode;
	u32	uid;
	u32	gid;
	u32	size;
	u32	last_access_time;
	u32	last_modifictation_time;
	u32	last_status_change_time;
	u32	nb_blocks_on_disk_allocated;
	u32	nb_hardlinks;
	u32*	blocks_on_disk;
};

typedef struct file_owner_t file_owner_t;
struct file_owner_t
{
	struct file_t*	file;
	spinlock_t	lock;
	u32		pid, uid, euid;
	s32		signum;
};

typedef struct file_credentials_t file_credentials_t;
struct file_credentials_t
{
	atomic_t	usage;
	u32		uid;
	u32		gid;
	u32		suid;
	u32		sgid;
	u32		euid;
	u32		egid;
	u32		fsuid;
	u32		fsgid;
};

/* Supports async buffered reads */
#define FILE_OPERATION_BUFFER_RASYNC	BIT0
/* Supports async buffered writes */
#define FILE_OPERATION_BUFFER_WASYNC	BIT1
/* Supports synchronous page faults for mappings */
#define FILE_OPERATION_MMAP_SYNC	BIT2
/* Supports non-exclusive O_DIRECT writes from multiple threads */
#define FILE_OPERATION_DIO_PARALLEL_W	BIT3
/* Contains huge pages */
#define FILE_OPERATION_HUGE_PAGE	BIT4
/* Treat loff_t as unsigned (e.g., /dev/mem) */
#define FILE_OPERATION_UNSIGNED_OFFSET	BIT5
/* Supports asynchronous lock callbacks */
#define FILE_OPERATION_ASYNC_LOCK	BIT6
/* File system supports uncached read/write buffered IO */
#define FILE_OPERATION_DONT_CACHE	BIT7

typedef struct file_operations_i file_operations_i;
struct file_operations_i
{
	u32	flags;
	s32	(*llseek)	(struct file_t*, s32, s32);
	s32	(*read)		(struct file_t*, char*, u32, s32*);
	s32	(*write)	(struct file_t*, char*, u32, s32*);
	s32	(*mmap)		(struct file_t*, struct mm_t*);
	s32	(*open)		(struct file_t*);
	s32	(*close)	(struct file_t*);
	s32	(*lock)		(struct file_t*);
};

/* file is open for reading */
#define	FILE_MODE_READ			BIT0
/* file is open for writing */
#define FILE_MODE_WRITE			BIT1
/* file is seekable */
#define FILE_MODE_LSEEK			BIT2
/* file can be accessed using pread */
#define FILE_MODE_PREAD			BIT3
/* file can be accessed using pwrite */
#define FILE_MODE_PWRITE		BIT4
/* File is opened for execution with sys_execve / sys_uselib */
#define FILE_MODE_EXEC			BIT5
/* File writes are restricted (block device specific) */
#define FILE_MODE_RESTRICTED		BIT6
/* File supports atomic writes */
#define FILE_MODE_ATOMIC_WRITE		BIT7

/* FMODE_* bit 8 (does not exist)*/

/* 32bit hashes as llseek() offset (for directories) */
#define FILE_MODE_32BIT_HASH		BIT9
/* 64bit hashes as llseek() offset (for directories) */
#define FILE_MODE_64BIT_HASH		BIT10
/*
 * Don't update ctime and mtime.
 *
 * Currently a special hack for the XFS open_by_handle ioctl, but we'll
 * hopefully graduate it to a proper O_CMTIME flag supported by open(2) soon.
 */
#define FILE_MODE_NO_CM_TIME		BIT11
/* Expect random access pattern */
#define FILE_MODE_RANDOM		BIT12
/* Supports IOCB_HAS_METADATA */
#define FILE_MODE_HAS_METADATA		BIT13
/* File is opened with O_PATH; almost nothing can be done with it */
#define FILE_MODE_PATH			BIT14
/* File needs atomic accesses to f_pos */
#define FILE_MODE_ATOMIC_POS		BIT15
/* Write access to underlying fs */
#define FILE_MODE_WRITER		BIT16
/* Has read method(s) */
#define FILE_MODE_CAN_READ		BIT17
/* Has write method(s) */
#define FILE_MODE_CAN_WRITE		BIT18
#define FILE_MODE_OPENED		BIT19
#define FILE_MODE_CREATED		BIT20
/* File is stream-like */
#define FILE_MODE_STREAM		BIT21
/* File supports DIRECT IO */
#define FILE_MODE_CAN_ODIRECT		BIT22
#define FILE_MODE_NO_REUSE		BIT23
/* File is embedded in backing_file object */
#define FILE_MODE_BACKING		BIT24
/*
 * Together with FMODE_NONOTIFY_PERM defines which fsnotify events shouldn't be
 * generated (see below)
 */
#define FILE_MODE_NO_NOTIFY		BIT25
/*
 * Together with FMODE_NONOTIFY defines which fsnotify events shouldn't be
 * generated (see below)
 */
#define FILE_MODE_NO_NOTIFY_PERM	BIT26
/* File is capable of returning -EAGAIN if I/O will block */
#define FILE_MODE_NO_WAIT		BIT27
/* File represents mount that needs unmounting */
#define FILE_MODE_NEED_UNMOUNT		BIT28
/* File does not contribute to nr_files count */
#define FILE_MODE_NO_ACCOUNT		BIT29

typedef struct file_t file_t;
struct file_t
{
// Note for private_data: socket_t data or directory_t data?
// TODO: look into f_pos_lock and FMODE_ATOMIC_POS and their relations with f_pipe
// https://github.com/torvalds/linux/blob/master/include/linux/fs.h#L1258
#define FILE_FIELDS(X)				\
	X(spinlock_t,		lock)		\
	X(u32, 			mode)		\
	X(u32, 			flags)		\
	X(inode_t, 		inode)		\
	X(void*,		private_data)	\
	X(file_operations_i*,	operations)	\
	X(struct mm_t*,		mapping)	\
	X(path_t,		path)		\
	X(file_owner_t*,	owner)		\
	X(file_credentials_t,	credentials)	\
	X(u32,			position)	\
	X(karena_t*,		arena)
	// file_ref
	FILE_FIELDS(SOA_DEFINE_FIELD)
};

SOA_DEFINE_STRUCT_OF_ARRAYS(file_t, FILE_FIELDS);

soa_file_t*	file_allocate_soa(karena_t* arena, u32 size);

typedef struct file_handle_t file_handle_t;
struct file_handle_t
{
	u32	handle_bytes_count;
	s32	handle_type;
	u8	handle[]; // __counted_by(handle_bytes_count);
};

void	fs_init(void);
file_t*	fs_get_root(void);
file_t*	fs_get_file(char* addr, u32 addr_len);

#endif
