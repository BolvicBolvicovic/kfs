#include <stdio.h>

static int
print(const char* data, size_t len)
{
	unsigned char* bytes = (unsigned char*)data;
	term_print(bytes, len);
	return 1;
}

int
printf(const char* restrict format, ...)
{
    va_list parameters;
    va_start(parameters, format);
    size_t  written = 0;
    while (*format)
	{
        size_t  max_size = INT_MAX - written;
        if (format[0] != '%' || format[1] == '%')
		{
            if (format[0] == '%') format++;
            size_t  to_be_written = 1;
            while (format[to_be_written] && format[to_be_written] != '%') ++to_be_written;
            if (to_be_written > max_size) return 1;
            if (!print(format, to_be_written)) return 1;
            format += to_be_written;
            written += to_be_written;
            continue;
        }
        const char* head = format++;
        size_t  len;
        switch (*format)
		{
            case 'c':
                format++;
                char c = (char)va_arg(parameters, int);
                if (!max_size) return 1;
                if (!print(&c, sizeof(c))) return 1;
                written++;
                break;
            case 's':
                format++;
                const char* s = va_arg(parameters, const char*);
                len = strlen(s);
                if (max_size < len) return 1;
                if (!print(s, len)) return 1;
                written += len;
                break;
            case 'd':
                format++;
                char    buf[16];
                int i = va_arg(parameters, int);
                len = itoa(buf, i);
                if (max_size < len) return 1;
                if (!print(buf, len)) return 1;
                written += len;
                break;
            case 'u':
                format++;
                char bufff[16];
                unsigned int k = va_arg(parameters, unsigned int);
                len = utoa(bufff, i);
                if (max_size < len) return 1;
                if (!print(bufff, len)) return 1;
                written += len;
            case 'x':
                format++;
                char    buff[16];
                unsigned int j = va_arg(parameters, unsigned int);
                len = itoxx(buf, j);
                if (max_size < len) return 1;
                if (!print(buf, len)) return 1;
                written += len;
                break;
            case 'p':
                format++;
                char aled[2 + 8 + 1] = "0";
                uint32_t ptr = va_arg(parameters, uint32_t);
                len = itox(aled + 2, ptr);
                if (max_size < len) return 1;
                if (!print(aled, len + 2)) return 1;
                written += len + 2;
                break;
            default:
                format = head;
                len = strlen(format);
                if (max_size < len) return 1;
                if (!print(format, len)) return 1;
                written += len;
                format += len;
        }
    }
    va_end(parameters);
    return written;
}
