#ifndef SYSCALL_H
# define SYSCALL_H

#include "../descriptor/descriptor.h"

# define SYSCALL_INTERRUPT_NUMBER 0x80
void init_syscall();

#endif
