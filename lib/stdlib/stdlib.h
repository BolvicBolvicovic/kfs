#ifndef STDLIB_H
#define STDLIB_H

#include <c_types.h>

s32	isnum(const char c);
u32	itoa(char* dest, s32 nb);
u32	utoa(char* dest, u32 nb);
u32	itox(char* dest, u32 nb);
u32	itoxx(char* dest, u32 nb);
s32	atoi(const char *nptr);

#endif
