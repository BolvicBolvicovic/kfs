#ifndef SOCKET_H
#define SOCKET_H

#include <c_types.h>
#include <memory/allocators/karena.h>
#include <processes/locks/spinlock.h>
#include <bits.h>

// Note must be a multiple of UINT_MAX + 1 so that when it overflows to 0,
// it matches the current index in the ringbuffer.
#define SOCK_MAX		128
#define SOCK_ADDR_SIZE_MAX	64
#define SOCK_QUEUE_SIZE		0x1000

#define SOCK_DOM_QUANT	1
enum
{
	AF_UNIX,
};

#define SOCK_TYPE_QUANT	1
enum
{
	SOCK_STREAM,
};


#define SOCK_PROT_QUANT 1
enum
{
	SOCK_PROT_DEFAULT = 0,
};

enum
{
	ERR_UNIX_ADDRINUSE = INT32_MIN,
	ERR_UNIX_BADF,
	ERR_UNIX_CONNREFUSED,
	ERR_UNIX_CONNRESET,
	ERR_UNIX_FAULT,
	ERR_UNIX_INVAL,
	ERR_UNIX_ISCONN,
	ERR_UNIX_NFILE,
	ERR_UNIX_NOENT,
	ERR_UNIX_NOMEM,
	ERR_UNIX_NOTCONN,
	ERR_UNIX_PNOTSUPP,
	ERR_UNIX_PERM,
	ERR_UNIX_PIPE,
	ERR_UNIX_PROTONOSUPPORT,
	ERR_UNIX_PROTOTYPE,
	ERR_UNIX_SOCKTNOSUPPORT,
	ERR_UNIX_SRCH,
	ERR_UNIX_TOOMANYREFS,
};

enum
{
	SOCK_BUCKET_UNIX_STREAM_DEF		= BIT0,
	SOCK_BUCKET_UNIX_STREAM_DEF_LISTEN	= BIT1,
};

typedef struct socket_t socket_t;
struct socket_t
{
	u32		addr_size;

	u8*		rcv;
	u32		read;
	u32		write;
	// Note: unnecessary ATM
//	u8*		snd;

	u32		id;
	s32		next;
	s32		prev;

	u32		bucket;

	spinlock_t	lock;
	s32		pair;

	karena_t*	arena;
};

s32	socket_new(s32 domain, s32 type, s32 protocol);
s32	socket_bind(s32 socket, char* addr, u32 addr_size);
s32	socket_listen(s32 socket, s32 backlog);
s32	socket_connect(s32 socket, char* addr, u32 addr_size);
s32	socket_accept(s32 socket, char* addr, u32* addr_size);
s32	socket_read(s32 socket, char* buffer, u32 buffer_size);
s32	socket_write(s32 socket, char* buffer, u32 buffer_size);
s32	socket_close(s32 socket);

#endif
