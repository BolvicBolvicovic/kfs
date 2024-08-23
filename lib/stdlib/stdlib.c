#include "stdlib.h"
inline int isnum(const char c) {
    return c >= 0x30 && c <= 0x39;
}

static inline size_t   nb_size(int nb) {
    size_t i = 0x1;
    while (nb > 0x9) { nb /= 0xA; i++; }
    return i;
}

size_t    itoa(char* dest, int nb) {
    size_t j = 0x0;
    bool   neg = false;
    if (nb < 0x0) { dest[j] = '-'; neg = true; nb = -nb; j++; }
    j += nb_size(nb);
    dest[j] = 0x0;
    for (size_t i = j; i-- > 0x0;) {
        if (!i && neg) break;
        dest[i] = nb % 0xA + 0x30;
        nb /= 0xA;
    }
    return j;
}

size_t  itox(char* dest, int nb) {
    char hex[] = "0123456789ABCDEF";
    for (size_t i = 7; i >= 0; i--) {
        dest[i] = hex[nb % 0x10];
        nb /= 0x10;
    }
    dest[8] = 0;
    return 8;
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
