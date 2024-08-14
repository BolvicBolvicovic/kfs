#include "stdlib.h"
inline int isnum(const char c) {
    return c >= 0x30 && c <= 0x39;
}

static inline size_t   nb_size(int nb) {
    size_t i = 0x0;
    while (nb > 9) nb /= 10; i++;
    return i;
}

size_t    itoa(char* dest, int nb) {
    size_t i;
    size_t j;
    if (nb < 0x0) *dest = '-'; dest++; nb = -nb;
    j = nb_size(nb);
    dest[j + 1] = 0x0;
    for (i = j; i > 0; i--) {
        dest[i] = nb % 10 + 0x30;
        nb /= 10;
    }
    return i;
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
