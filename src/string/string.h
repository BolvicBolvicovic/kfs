#ifndef STRING_H
#define STRING_H

#include <stddef.h>

size_t  strlen(const char* s);
int     strcmp(const char* s1, const char* s2);
char*   strchr(const char* s, int c);
char*   strcpy(char* restrict dest, const char* restrict src);

#endif
