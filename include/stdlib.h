#ifndef STDLIB_H
#define STDLIB_H

#include <c_types.h>

#define	ALIGN(x, b)		(((x) + (b) - 1)&(~((b) - 1)))
#define ALIGN_DOWN(x, b)	((x)&(~((b) - 1)))
#define MIN(a, b)		((a) < (b) ? (a) : (b))
#define MAX(a, b)		((a) > (b) ? (a) : (b))
#define CLAMP_TOP(a, x)		MIN(a, x)
#define CLAMP_BOT(x, b)		MAX(x, b)

s32	isnum(const char c);
u32	itoa(char* dest, s32 nb);
u32	utoa(char* dest, u32 nb);
u32	itox(char* dest, u32 nb);
u32	itoxx(char* dest, u32 nb);
s32	atoi(const char *nptr);

#endif
