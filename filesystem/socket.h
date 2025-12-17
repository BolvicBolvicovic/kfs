#ifndef SOCKET_H
#define SOCKET_H

#include "fs.h"

enum
{
	AF_UNIX		= 0,
	AF_INET		= 1,
	AF_INET6	= 2,
};

enum
{
	SOCK_RAW	= 0,
	SOCK_STREAM	= 1,
	SOCK_DGRAM	= 2,
};

enum
{
	SOCK_IDLE,
	SOCK_CONNECTED,
	SOCK_LISTENING,
};

enum
{
	SOCK_CONNECT_ERROR_NOT_FOUND,
	SOCK_CONNECT_ERROR_NOT_A_SOCKET,
	SOCK_CONNECT_ERROR_NOT_LISTENING,
	SOCK_CONNECT_ERROR_WRONG_TYPE,	
};

typedef struct socket_message_t socket_message_t;
struct socket_message_t
{
	socket_messages_t*	next;
	u8*			bytes;
};

#define SOCKET_RS_LIST_SIZE_DEFAULT 100

LINKED_LIST_STRUCT(socket_messages_t, socket_message_t);

struct net_protocol_operations_i;

typedef struct socket_t socket_t;
struct socket_t
{
	u16					state;
	u16					type;
	u32					protocol;
	file_t*					file;
	socket_messages_t			receive_list;
	socket_messages_t			send_list;
	spinlock_t				receive_list_lock;
	spinlock_t				send_list_lock;
	struct net_protocol_operations_i	operations;
	void*					protocol_data;
	u32					receive_list_buffer_size;
	u32					send_list_buffer_size;
};

enum
{
	PROTOCOL_UNIX,
	PROTOCOL_RAW,
	PROTOCOL_TCP,
	PROTOCOL_UDP,
};

typedef struct net_protocol_operations_i net_protocol_operations_i;
struct net_protocol_operations_i
{
	s32	family;
	s32	(*bind)		(socket_t*, char* addr, u32 addr_len);
	s32	(*connect)	(socket_t*, char* addr, u32 addr_len, u32 flags);
	s32	(*accept)	(socket_t*, socket_t* new, net_protocol_accept_args*);
	s32	(*listen)	(socket_t*, u32 len);
	s32	(*sendmsg)	(socket_t*, char* msg, u32 msg_len);
	s32	(*recvmsg)	(socket_t*, char* msg, u32 msg_len, u32 flags);
	s32	(*mmap)		(socket_t*, mm_t*);
};

s32	socket_new(s32 family, u16 type, u32 protocol);
void	socket_send();
void	socket_receive();
void	socket_request();

#endif
