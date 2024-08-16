#ifndef UTILS_H
#define UTILS_H

inline unsigned char port_byte_in(unsigned short port) {
    unsigned char result;
    asm("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

inline void port_byte_out(unsigned short port, unsigned char data) {
    asm("out %%al, %%dx" : : "a" (data), "d" (port));
}

#endif
