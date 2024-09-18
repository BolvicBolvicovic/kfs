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

inline void insl(unsigned short port, void *buffer, unsigned int count) {
    asm volatile (
        "rep insl"                // Repeat the 'insl' instruction 'count' times
        : "+D"(buffer), "+c"(count)  // '+D' is the destination (EDI in x86), '+c' is the counter (ECX in x86)
        : "d"(port)               // 'd' is the source port (DX in x86)
        : "memory"                // Inform the compiler that memory is affected
    );
}

#endif
