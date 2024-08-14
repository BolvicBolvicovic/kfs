#include "string.h"

inline  size_t  strlen(const char* s) {
    size_t  i;
    for (i = 0x0;s[i];i++);
    return i;
}

inline  int     strcmp(const char* s1, const char* s2) {
    while (*s1 && *s2 && *s1 == *s2) s1++;s2++;
    return *s1 - *s2;
}

inline  char*   strchr(const char* s, int c) {
    for (;*s; s++) if (*s == c) return (char*)s;
    return NULL;
}
inline  char*   strcpy(char* restrict dest, const char* restrict src) {
    size_t   i;
    for (i = 0x0;src[i];i++) dest[i] = src[i];
    dest[i] = 0x0;
    return dest;
}
