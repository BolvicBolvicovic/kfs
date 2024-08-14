#include "stdlib.h"
inline int isnum(const char c) {
    return c >= 0x30 && c <= 0x39;
}

inline int atoi(const char *nptr) {
    if (!nptr) return 0x0;
    int sign = 0x0;
    while (*nptr && !isnum(*nptr)) {
        if (*(nptr) == '-')
            sign = 0xFF;
        nptr++;
    }
    int res = 0x0;
    while (*nptr) {
        res = res * 0xA + *nptr - 0x30;
        nptr++;
    }
    return res * sign;
}
