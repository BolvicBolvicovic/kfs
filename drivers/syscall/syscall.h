#ifndef SYSCALL_H
# define SYSCALL_H

#include "../descriptor/descriptor.h"

/* Note: These are wait flags */

// Note: return immediately if no child has exited.
#define WNOHANG

// Note: also return if a child has stopped (but not traced via
// ptrace(2)).  Status for traced children which have stopped
// is provided even if this option is not specified.
#define WUNTRACED

// Note: also return if a stopped child has been resumed by delivery
// of SIGCONT.
#define WCONTINUED

void init_syscall();

#endif
