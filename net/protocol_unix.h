#ifndef PROTOCOL_UNIX_H
#define PROTOCOL_UNIX_H

#include <filesystem/socket.h>

s32	unix_release(socket_t*);
s32	unix_bind(socket_t*, char* addr, u32 addr_len);
s32	unix_connect(socket_t*, char* addr, u32 addr_len, u32 flags);
s32	unix_accept(socket_t*, socket_t* new, net_protocol_accept_args*);
s32	unix_listen(socket_t*, u32 len);
s32	unix_sendmsg(socket_t*, char* msg, u32 msg_len);
s32	unix_recvmsg(socket_t*, char* msg, u32 msg_len, u32 flags);
s32	unix_mmap(socket_t*, mm_t*);

#endif
