#ifndef STRING_H
#define STRING_H

#include <stddef.h>

size_t  strlen(const char* s);
int     strcmp(const char* s1, const char* s2);
char*   strchr(const char* s, int c);
void	memcpy(void* dest, const void* src, size_t n);
char*   strcpy(char* dest, const char* src);

#endif
