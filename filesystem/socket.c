#include "socket.h"
#include "fdtable.h"
#include <memory/allocators/karena.h>
#include <net/protocol_unix.h>

static s32	sock_read(file_t*, char*, u32, s32*);
static s32	sock_write(file_t*, char*, u32, s32*);
static s32	sock_mmap(file_t*, struct mm_t*);
static s32	sock_close(file_t*);

static net_protocol_operations_i	protocols[] =
{
	{
		.family		= PROTOCOL_UNIX,
		.release	= unix_release,
		.bind		= unix_bind,
		.connect	= unix_connect,
		.accept		= unix_accept,
		.listen		= unix_listen,
		.sendmsg	= unix_sendmsg,
		.recvmsg	= unix_recvmsg,
		.mmap		= unix_mmap,
	},
	{
		.family	= PROTOCOL_RAW,
		// TODO: implement protocol
	},
	{
		.family	= PROTOCOL_TCP,
		// TODO: implement protocol
	},
	{
		.family	= PROTOCOL_UDP,
		// TODO: implement protocol
	},
};

static file_operations_i	socket_operations =
{
	.flags	= FILE_TYPE_SOCKET,
	.read	= sock_read,
	.write	= sock_write,
	.mmap	= sock_mmap,
	.close	= sock_close,
};

static s32
sock_read(file_t* file, char* buffer, u32 count, s32* offset)
{
	(void)offset;
	socket_t*	socket	= file->private_data;
	s32		res	= socket->operations->recvmsg(socket, buffer, count, 0);

	return res;
}

static s32
sock_write(file_t* file, char* buffer, u32 count, s32* offset)
{
	(void)offset;
	socket_t*	socket	= file->private_data;
	s32		res	= socket->operations->sendmsg(socket, buffer, count);

	return res;
}

static s32
sock_mmap(file_t* file, struct mm_t* mm)
{
	socket_t*	socket = file->private_data;
	
	return socket->operations->mmap(socket, mm);
}

static s32
sock_close(file_t* file)
{
	socket_t*	socket	= file->private_data;
	s32		res	= socket->operations->release(socket);

	karena_release(file->arena);

	return res;
}

s32
socket_new(s32 family, u16 type, u32 protocol)
{
	karena_t*	arena	= KARENA_ALLOC();

	if (!arena)
		return -1;

	s32	fd = fd_get(0);

	if (fd == -1)
	{
		karena_release(arena);
		return -1;
	}

	net_protocol_operations_i*	protocol_operations	= &protocols[family];
	socket_t*			socket			= KARENA_PUSH_STRUCT(arena, socket_t);
	file_t*				file			= KARENA_PUSH_STRUCT(arena, file_t);

	file->arena				= arena;
	socket->file				= file;
	socket->state				= SOCK_IDLE;
	socket->type				= type;
	socket->protocol			= protocol;
	socket->operations			= protocol_operations;
	socket->type				= type;
	socket->receive_list.head		= KARENA_PUSH_ARRAY(arena,
								socket_message_t,
								SOCKET_RS_LIST_SIZE_DEFAULT);
	socket->receive_list.tail		= socket->receive_list.head;
	socket->send_list.head			= KARENA_PUSH_ARRAY(arena,
								socket_message_t,
								SOCKET_RS_LIST_SIZE_DEFAULT);
	socket->send_list.tail			= socket->send_list.head;
	socket->receive_list_lock.counter	= 0;
	socket->send_list_lock.counter		= 0;
	socket->receive_list_buffer_size	= SOCKET_RS_LIST_SIZE_DEFAULT;
	socket->send_list_buffer_size		= SOCKET_RS_LIST_SIZE_DEFAULT;

	// TODO: set up file
	file->lock.counter	= 0;
	// TODO: add steaming && direct IO
	file->mode		= FILE_MODE_CAN_READ | FILE_MODE_CAN_WRITE; 
	file->flags		= 0;
	file->inode.mode	= FILE_TYPE_SOCKET | FILE_PERMISSION_RWXU;
	file->private_data	= &socket;
	file->operations	= &socket_operations;
	// TODO: handle file mapping
	file->mapping		= 0;
	// Note: file->path is initilized when binding socket to a path.
	// TODO: look up how to use/set up file->owner & file->credentials
	file->position		= 0;

	fd_install(fd, file);

	return fd;
}
