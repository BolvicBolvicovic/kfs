#include "processes.h"

pid_t
fork(void)
{
	pid_t	res = 0;
	
	asm volatile (
		"mov $57, %%eax\n\t"
		"int $0x80\n\t"
		: "=a"(res)
		:: "memory"
	);

	return res;
	
}

pid_t
wait(int* wstatus)
{
	pid_t	res = 0;
	
	asm volatile (
		"mov $247, %%eax\n\t"
		"int $0x80\n\t"
		: "=a"(res)
		: "D"(wstatus)
		: "memory"
	);

	return res;
}

void
exit(int status)
{
	asm volatile (
		"mov $60, %%eax\n\t"
		"int $0x80\n\t"
		:: "D"(status)
		: "memory"
	);

	return;
}

uid_t
getuid(void)
{
	uid_t	res;

	asm volatile (
		"mov $102, %%eax\n\t"
		"int $0x80\n\t"
		: "=a"(res)
		:: "memory"
	);

	return res;
}

//sighandler_t	signal(int signum, sighandler_t handler);

int
kill(pid_t pid, int signal)
{
	int	res;

	asm volatile (
		"mov $62, %%eax\n\t"
		"int $0x80\n\t"
		: "=a"(res)
		: "D"(pid), "S"(signal)
		: "memory"
	);

	return res;
}
