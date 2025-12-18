#ifndef STRING_H
#define STRING_H

#include <c_types.h>
#include <compiler.h>

typedef	uint32_t __may_alias word;

#define LSB 			((u32)-1 / UCHAR_MAX)
#define REPEAT_BYTE_IN_WORD(a)	((word)((u8)(a) * LSB))

u32	strlen(const char* s);
s32	strcmp(const char* s1, const char* s2);
char*	strchr(const char* s, int c);
void*	memcpy(void* dest, const void* src, u32 n);
s32	memcmp(const void* b1, const void* b2, u32 n);
char*	strcpy(char* dest, const char* src);
void*	memset(void* s, u8 c, u32 n);

#endif
