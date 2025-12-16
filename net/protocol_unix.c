#include "protocol_raw.h"

s32
unix_release(socket_t* socket)
{
	(void)socket;
	return 0;
}

s32
unix_bind(socket_t* socket, char* addr, u32 addr_len)
{
	file_t*	file = socket->file;

	// TODO: create a path.h and path.c in filesystem folder in which the path_t is handled;
	
	return 0;
}

s32
unix_connect(socket_t* socket, char* addr, u32 addr_len, u32 flags)
{
	// TODO: socket_get(addr, addr_len) && try to connect
	return 0;
}

s32
unix_accept(socket_t* socket, socket_t* new, net_protocol_accept_args* args)
{
	// TODO: find out how we know that a certain socket tries to connect
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
unix_mmap(socket_t* socket, mm_t* mm)
{
	// TODO: might need to do something here
	return 0;
}
