#include "socket.h"
#include <bitmap.h>
#include <string.h>
#include <stdio.h>

static socket_t		_sockets[SOCK_MAX]			= { 0 };
static s32		_sockets_unix_stream_def_head		= -1;
static spinlock_t	_sockets_unix_stream_def_lock 		= { 0 };
static s32		_sockets_unix_stream_def_listen_head	= -1;
static spinlock_t	_sockets_unix_stream_def_listen_lock 	= { 0 };

// TODO: move addresses to the filesystem path
static char		_sockets_addresses[SOCK_ADDR_SIZE_MAX * SOCK_MAX] = { 0 };

static u32		_sockets_map[SOCK_MAX / BITMAP_CHUNK_SIZE] = { 0 };
static spinlock_t	_sockets_lock	= { 0 };
static bitmap_t		_sockets_bm	=
{
	.map = _sockets_map,
	.size = SOCK_MAX,
	.position = 0
};

s32
socket_new(s32 domain, s32 type, s32 protocol)
{
	if (domain < 0 || domain >= SOCK_DOM_QUANT)
		return ERR_UNIX_INVAL;

	if (type < 0 || type >= SOCK_TYPE_QUANT)
		return ERR_UNIX_SOCKTNOSUPPORT;

	if (protocol < 0 || protocol >= SOCK_PROT_QUANT)
		return ERR_UNIX_PROTONOSUPPORT;

	s32	id = ERR_UNIX_FAULT;

	spinlock_lock(&_sockets_lock);

	if ((id = bitmap_find_next_free_bit(&_sockets_bm)) == -1)
	{
		spinlock_unlock(&_sockets_lock);
		return ERR_UNIX_NOMEM;
	}
	
	bitmap_set_bit(&_sockets_bm, id);		
	
	spinlock_unlock(&_sockets_lock);
	
	_sockets[id].addr_size		= 0;
	_sockets[id].pair		= -1;
	_sockets[id].arena		= KARENA_ALLOC();
	_sockets[id].rcv		= KARENA_PUSH_ARRAY(_sockets[id].arena, u8, SOCK_QUEUE_SIZE);
	_sockets[id].id			= id;
	_sockets[id].prev		= -1;
	_sockets[id].lock.counter	= 0;
		

	if (domain == AF_UNIX && type == SOCK_STREAM && protocol == SOCK_PROT_DEFAULT)
	{
		_sockets[id].bucket	= SOCK_BUCKET_UNIX_STREAM_DEF;

		spinlock_lock(&_sockets_unix_stream_def_lock);

		if (_sockets_unix_stream_def_head != -1)
			_sockets[_sockets_unix_stream_def_head].prev = id;

		_sockets[id].next		= _sockets_unix_stream_def_head;
		_sockets_unix_stream_def_head	= id;

		spinlock_unlock(&_sockets_unix_stream_def_lock);
	}

	return id;
}

s32
socket_bind(s32 socket, char* addr, u32 addr_size)
{
	if (socket < 0 || !addr || addr_size >= SOCK_ADDR_SIZE_MAX)
		return ERR_UNIX_INVAL;

	if (!bitmap_test_bit(&_sockets_bm, socket))
		return ERR_UNIX_NOENT;

	for (u32 i = 0; i < SOCK_MAX; i++)
	{
		if (_sockets[i].addr_size != addr_size)
			continue;

		char*	sock_addr = _sockets_addresses + i * SOCK_ADDR_SIZE_MAX;

		if (strcmp(sock_addr, addr) == 0)
			return ERR_UNIX_ADDRINUSE;
	}
	
	char*	sock_addr = _sockets_addresses + socket * SOCK_ADDR_SIZE_MAX;

	memcpy(sock_addr, addr, addr_size);
	_sockets[socket].addr_size = addr_size;

	return 0;
}

s32
socket_listen(s32 socket, s32 backlog)
{
	(void)backlog;

	if (socket < 0)
		return ERR_UNIX_INVAL;

	if (!bitmap_test_bit(&_sockets_bm, socket))
		return ERR_UNIX_NOENT;

	if (!(_sockets[socket].bucket & SOCK_BUCKET_UNIX_STREAM_DEF))
		return ERR_UNIX_ISCONN;
	
	_sockets[socket].bucket = SOCK_BUCKET_UNIX_STREAM_DEF_LISTEN;

	spinlock_lock(&_sockets_unix_stream_def_lock);

	if (socket == _sockets_unix_stream_def_head)
		_sockets_unix_stream_def_head		= _sockets[socket].next;
	else
		_sockets[_sockets[socket].prev].next	= _sockets[socket].next;

	if (_sockets[socket].next != -1)
		_sockets[_sockets[socket].next].prev	= _sockets[socket].prev;

	spinlock_unlock(&_sockets_unix_stream_def_lock);

	spinlock_lock(&_sockets_unix_stream_def_listen_lock);

	_sockets[socket].next = _sockets_unix_stream_def_listen_head;
	_sockets[socket].prev = -1;
	_sockets_unix_stream_def_listen_head = socket;

	spinlock_unlock(&_sockets_unix_stream_def_listen_lock);

	return 0;
}

s32
socket_connect(s32 socket, char* addr, u32 addr_size)
{
	if (socket < 0)
		return ERR_UNIX_INVAL;

	if (!bitmap_test_bit(&_sockets_bm, socket))
		return ERR_UNIX_NOENT;
	
	if (!(_sockets[socket].bucket & SOCK_BUCKET_UNIX_STREAM_DEF))
		return ERR_UNIX_ISCONN;

	// TODO: add a perm check when filesystem is ON
	bool	is_valid_conn = 0;

	spinlock_lock(&_sockets_unix_stream_def_listen_lock);

	s32	sock = _sockets_unix_stream_def_listen_head;

	for (; sock != -1; sock = _sockets[sock].next)
	{
		if (_sockets[sock].addr_size != addr_size)
			continue;

		char*	conn_addr = _sockets_addresses + SOCK_ADDR_SIZE_MAX * sock;

		if (strcmp(conn_addr, addr) == 0)
		{
			is_valid_conn = 1;
			break;
		}
	}
	
	spinlock_unlock(&_sockets_unix_stream_def_listen_lock);

	if (!is_valid_conn)
		return ERR_UNIX_NOTCONN;

	spinlock_lock(&_sockets[sock].lock);

	_sockets[sock].pair = socket;

	// Note: I am not unlocking there, but I do in accept
	// I don't want to block the client
	// However, I could make a queue to be even less blocking
	return 0;
}

s32
socket_accept(s32 socket, char* addr, u32* addr_size)
{
	if (socket < 0)
		return ERR_UNIX_INVAL;
	
	if (!(_sockets[socket].bucket & SOCK_BUCKET_UNIX_STREAM_DEF_LISTEN))
		return ERR_UNIX_NOENT;

	if (_sockets[socket].addr_size != -1) 
	{
		if (addr_size)
			*addr_size = _sockets[socket].addr_size;
		
		if (addr)
			memcpy(addr, _sockets_addresses + SOCK_ADDR_SIZE_MAX * socket, _sockets[socket].addr_size);
	}

	if (_sockets[socket].pair == -1)
		return 0;
	
	s32	new_conn = -1; 

	if (_sockets[socket].bucket & SOCK_BUCKET_UNIX_STREAM_DEF_LISTEN)
		new_conn = socket_new(AF_UNIX, SOCK_STREAM, 0);

	if (new_conn < 0)
		return new_conn;

	_sockets[_sockets[socket].pair].pair = new_conn;
	_sockets[new_conn].pair = _sockets[socket].pair;

	_sockets[socket].pair = -1;
	
	// Note: unlock connect lock
	spinlock_unlock(&_sockets[socket].lock);
	
	return new_conn;
}

s32
socket_read(s32 socket, char* buffer, u32 buffer_size)
{
	if (socket < 0 || !buffer)
		return ERR_UNIX_INVAL;

	if (_sockets[socket].bucket & SOCK_BUCKET_UNIX_STREAM_DEF_LISTEN)
		return ERR_UNIX_NOTCONN;
	
	spinlock_lock(&_sockets[socket].lock);

	s32	w_size = buffer_size < _sockets[socket].write - _sockets[socket].read
			? buffer_size
			: _sockets[socket].write - _sockets[socket].read;

	memcpy(buffer, _sockets[socket].rcv + _sockets[socket].read, w_size);
	_sockets[socket].read += w_size;
	spinlock_unlock(&_sockets[socket].lock);

	return w_size;
}

s32
socket_write(s32 socket, char* buffer, u32 buffer_size)
{
	if (socket < 0)
		return ERR_UNIX_INVAL;
	
	s32	pair = _sockets[socket].pair;

	if (pair == -1 || _sockets[socket].bucket & SOCK_BUCKET_UNIX_STREAM_DEF_LISTEN)
		return ERR_UNIX_NOTCONN;

	spinlock_lock(&_sockets[pair].lock);

	s32	w_size = buffer_size < SOCK_QUEUE_SIZE - _sockets[pair].write
			? buffer_size
			: SOCK_QUEUE_SIZE - _sockets[pair].write;

	memcpy(_sockets[pair].rcv + _sockets[pair].write, buffer, w_size);
	_sockets[pair].write += w_size;
	spinlock_unlock(&_sockets[pair].lock);

	return w_size;
}

s32
socket_close(s32 socket)
{
	if (socket < 0)
		return ERR_UNIX_INVAL;

	if (_sockets[socket].bucket & SOCK_BUCKET_UNIX_STREAM_DEF)
	{
		spinlock_lock(&_sockets_unix_stream_def_lock);

		if (socket == _sockets_unix_stream_def_head)
			_sockets_unix_stream_def_head		= _sockets[socket].next;
		else
			_sockets[_sockets[socket].prev].next	= _sockets[socket].next;

		if (_sockets[socket].next != -1)
			_sockets[_sockets[socket].next].prev	= _sockets[socket].prev;

		spinlock_unlock(&_sockets_unix_stream_def_lock);
	}

	if (_sockets[socket].bucket & SOCK_BUCKET_UNIX_STREAM_DEF_LISTEN)
	{
		spinlock_lock(&_sockets_unix_stream_def_listen_lock);

		if (socket == _sockets_unix_stream_def_listen_head)
			_sockets_unix_stream_def_listen_head	= _sockets[socket].next;
		else
			_sockets[_sockets[socket].prev].next	= _sockets[socket].next;

		if (_sockets[socket].next != -1)
			_sockets[_sockets[socket].next].prev	= _sockets[socket].prev;

		spinlock_unlock(&_sockets_unix_stream_def_listen_lock);
	}

	_sockets[socket].addr_size	= 0;
	_sockets[socket].pair		= -1;
	_sockets[socket].bucket		= 0;
	_sockets[socket].prev		= -1;
	_sockets[socket].lock.counter	= 0;
	karena_release(_sockets[socket].arena);

	return 0;
}
