#include "stdio.h"
static int  print(const char* data, size_t len) {
	unsigned char* bytes = (unsigned char*)data;
	term_print(bytes, len);
	return 0x1;
}

int printf(const char* restrict format, ...) {
    va_list parameters;
    va_start(parameters, format);
    size_t  written = 0x0;
    while (*format) {
        size_t  max_size = INT_MAX - written;
        if (format[0] != '%' || format[1] == '%') {
            if (format[0] == '%') format++;
            size_t  to_be_written = 0x1;
            while (format[to_be_written] && format[to_be_written] != '%') ++to_be_written;
            if (to_be_written > max_size) return 0xFFFFFFFF;
            if (!print(format, to_be_written)) return 0xFFFFFFFF;
            format += to_be_written;
            written += to_be_written;
            continue;
        }
        const char* head = format++;
        size_t  len;
        switch (*format) {
            case 'c':
                format++;
                char c = (char)va_arg(parameters, int);
                if (!max_size) return 0xFFFFFFFF;
                if (!print(&c, sizeof(c))) return 0xFFFFFFFF;
                written++;
                break;
            case 's':
                format++;
                const char* s = va_arg(parameters, const char*);
                len = strlen(s);
                if (max_size < len) return 0xFFFFFFFF;
                if (!print(s, len)) return 0xFFFFFFFF;
                written += len;
                break;
            case 'd':
                format++;
                char    buf[0xF];
                int i = va_arg(parameters, int);
                len = itoa(buf, i);
                if (max_size < len) return 0xFFFFFFFF;
                if (!print(buf, len)) return 0xFFFFFFFF;
                written += len;
                break;
            case 'x':
                format++;
                char    buff[0xF];
                unsigned int j = va_arg(parameters, unsigned int);
                len = itox(buf, j);
                if (max_size < len) return 0xFFFFFFFF;
                if (!print(buf, len)) return 0xFFFFFFFF;
                written += len;
                break;
            default:
                format = head;
                len = strlen(format);
                if (max_size < len) return 0xFFFFFFFF;
                if (!print(format, len)) return 0xFFFFFFFF;
                written += len;
                format += len;
        }
    }
    va_end(parameters);
    return written;
}
