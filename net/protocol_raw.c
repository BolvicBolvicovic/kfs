#include "protocol_raw.h"

s32
pr_release(socket_t* socket)
{
}

s32
pr_bind(socket_t* socket, char* addr, u32 addr_len)
{
}

s32
pr_connect(socket_t* socket, char* addr, u32 addr_len, u32 flags)
{
}

s32
pr_accept(socket_t* socket, socket_t* new, net_protocol_accept_args*)
{
}

s32
pr_listen(socket_t* socket, u32 len)
{

}

s32
pr_sendmsg(socket_t* socket, char* msg, u32 msg_len)
{
}

s32
pr_recvmsg(socket_t* socket, char* msg, u32 msg_len, u32 flags)
{

}

s32
pr_mmap(socket_t* socket, mm_t* mm)
{

}
