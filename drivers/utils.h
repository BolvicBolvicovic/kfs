#ifndef UTILS_H
#define UTILS_H


inline unsigned char port_byte_in(unsigned short port) {
    unsigned char result;
    asm volatile("inb %1, %0" : "=a" (result) : "dN" (port));
    return result;
}

inline void port_byte_out(unsigned short port, unsigned char data) {
    asm volatile("outb %1, %0" : : "dN" (port), "a" (data));
}

#endif
