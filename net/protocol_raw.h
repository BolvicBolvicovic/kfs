#ifndef PROTOCOL_RAW_H
#define PROTOCOL_RAW_H

#include <filesystem/socket.h>

s32	pr_release(socket_t*);
s32	pr_bind(socket_t*, char* addr, u32 addr_len);
s32	pr_connect(socket_t*, char* addr, u32 addr_len, u32 flags);
s32	pr_accept(socket_t*, socket_t* new, net_protocol_accept_args*);
s32	pr_listen(socket_t*, u32 len);
s32	pr_sendmsg(socket_t*, char* msg, u32 msg_len);
s32	pr_recvmsg(socket_t*, char* msg, u32 msg_len, u32 flags);
s32	pr_mmap(socket_t*, mm_t*);

#endif
