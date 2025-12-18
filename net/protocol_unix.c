#include "protocol_unix.h"
#include <filesystem/socket.h>
#include <string.h>

s32
unix_release(socket_t* socket)
{
	(void)socket;
	return 0;
}

s32
unix_bind(socket_t* socket, char* addr, u32 addr_len)
{
	file_t*			file = socket->file;
	karena_t*		arena= file->arena;
	unix_sock_data_t*	data = socket->protocol_data;
	u32			err  = path_bind(arena, &file->path, addr, addr_len);

	if (err)
		return err;

	data->unix_addr = KARENA_PUSH_ARRAY(arena, char, addr_len);
	memcpy(data->unix_addr, addr, addr_len);
	data->unix_addr_len = addr_len;
	
	return 0;
}

s32
unix_connect(socket_t* socket, char* addr, u32 addr_len, u32 flags)
{
	unix_sock_data_t*	data		= socket->protocol_data;
	file_t*			pair_file	= fs_get_file(addr, addr_len);

	if (!pair_file)
		return SOCK_CONNECT_ERROR_NOT_FOUND;

	if (pair_file->inode.mode != FILE_TYPE_SOCKET)
		return SOCK_CONNECT_ERROR_NOT_A_SOCKET;

	socket_t*	pair = (socket_t*)pair_file->private_data;

	// TODO: check flags to define nature of connection
	(void)flags;
	
	if (pair->type != AF_UNIX)
		return SOCK_CONNECT_ERROR_WRONG_TYPE;

	if (pair->state != SOCK_LISTENING)
		return SOCK_CONNECT_ERROR_NOT_LISTENING;
	
	data->pair	= pair;
	pair->state	= SOCK_CONNECTED;
	socket->state	= SOCK_CONNECTED;

	return 0;
}

s32
unix_accept(socket_t* socket, socket_t* new, net_protocol_accept_args* args)
{
	// TODO: find out how we know that a certain socket tries to connect
	// TODO: do something with args:
	// https://github.com/torvalds/linux/blob/master/include/net/sock.h#L1268
	(void)args;
	return 0;
}

s32
unix_listen(socket_t* socket, u32 len)
{
	// TODO: find why there is a len
	return 0;
}

s32
unix_sendmsg(socket_t* socket, char* msg, u32 msg_len)
{
	// TODO: look how to get the socket we are connected to
	return 0;
}

s32
unix_recvmsg(socket_t* socket, char* msg, u32 msg_len, u32 flags)
{
	// TODO: might need to do something here
	return 0;
}

s32
unix_mmap(socket_t* socket, struct mm_t* mm)
{
	// TODO: might need to do something here
	return 0;
}
