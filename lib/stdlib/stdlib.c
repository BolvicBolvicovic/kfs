#include "stdlib.h"
inline int isnum(const char c) {
    return c >= 0x30 && c <= 0x39;
}

static inline size_t   nb_size(int nb) {
    size_t i = 0x1;
    while (nb > 0x9) { nb /= 0xA; i++; }
    return i;
}

static inline size_t   unb_size(unsigned int nb) {
    size_t i = 1;
    while (nb >= 10) { nb /= 10; i++; }
    return i;
}

size_t    itoa(char* dest, int nb) {
    size_t j = 0x0;
    bool   neg = false;
    if (nb < 0x0) { dest[j] = '-'; neg = true; nb = -nb; j++; }
    j += nb_size(nb);
    dest[j] = 0x0;
    for (int i = j; i-- > 0x0;) {
        if (!i && neg) break;
        dest[i] = nb % 0xA + 0x30;
        nb /= 0xA;
    }
    return j;
}

size_t    utoa(char* dest, unsigned int nb) {
    size_t j = 0;
    j += unb_size(nb);
    dest[j] = '\0';
    for (int i = j; i-- > 0;) {
        dest[i] = nb % 10 + '0';
        nb /= 10;
    }
    return j;
}

size_t  itox(char* dest, unsigned int nb) {
    char hex[] = "0123456789ABCDEF";
    char *itr = dest + 7;
    while (itr >= dest) {
        *itr-- = hex[nb % 0x10];
        nb /= 0x10;
    }
    dest[8] = 0;
    return 8;
}

size_t  _itoxx(char *dest, unsigned int n) {
    char    hex[] = "0123456789ABCDEF";
    *dest = hex[n % 0x10];
    n /= 0x10;
    if (n != 0)
        return (1 + _itoxx(dest - 1, n));
    return (1);
}

size_t  itoxx(char *dest, unsigned int n) {
    size_t size = 0;
    unsigned int caca = n;
    while (caca) {
        caca /= 0x10;
        ++size;
    }
    dest[size] = '\0';
    return (_itoxx(dest + size - 1, n));
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
