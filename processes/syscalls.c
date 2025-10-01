#include "processes.h"

pid_t
wait(int* wstatus)
{
	pid_t	res = 0;
	
	asm volatile (
		"syscall"
		: "=a"(res)
		: "D"(wstatus), "a"(247)
	);

	return res;
}

void
exit(int status)
{
	asm volatile (
		"syscall"
		:
		: "D"(status), "a"(60)
	);

	return;
}

uid_t
getuid(void)
{
	uid_t	res;

	asm volatile (
		"syscall"
		: "=a"(res)
		: "a"(102)
	);

	return res;
}

//sighandler_t	signal(int signum, sighandler_t handler);

int
kill(pid_t pid, int signal)
{
	int	res;

	asm volatile (
		"syscall"
		: "=a"(res)
		: "D"(pid), "S"(signal), "a"(62)
	);

	return res;
}
