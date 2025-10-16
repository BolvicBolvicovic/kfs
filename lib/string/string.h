#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

typedef	uint32_t __attribute__((may_alias)) word;

#ifndef UCHAR_MAX
#define UCHAR_MAX				255
#endif
#define LSB 					((uint32_t)-1 / UCHAR_MAX)
#define REPEAT_BYTE_IN_WORD(a)	((word)((uint8_t)(a) * LSB))


size_t  strlen(const char* s);
int     strcmp(const char* s1, const char* s2);
char*   strchr(const char* s, int c);
void*	memcpy(void* dest, const void* src, size_t n);
char*   strcpy(char* dest, const char* src);
void*   memset(void* s, uint8_t c, size_t n);

#endif
