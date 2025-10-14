#include "string.h"

inline size_t
strlen(const char* s)
{
    size_t  i;
    for (i = 0x0;s[i];i++);
    return i;
}

inline int
strcmp(const char* s1, const char* s2)
{
    while (*s1 && *s2 && *s1 == *s2) { s1++; s2++; }
    return *s1 - *s2;
}

inline char*
strchr(const char* s, int c)
{
    for (;*s; s++) if (*s == c) return (char*)s;
    return NULL;
}

inline char*
strcpy(char* restrict dest, const char* restrict src)
{
    size_t   i;
    for (i = 0x0;src[i];i++) dest[i] = src[i];
    dest[i] = 0x0;
    return dest;
}

inline void
memcpy(void* dest, const void* src, size_t n)
{
	unsigned char* d = (unsigned char*)dest;
	unsigned char* s = (unsigned char*)src;
	for (size_t i = 0; i < n; i++)
	{
		d[i] = s[i];
	}
}

void*
memset(void* s, uint8_t c, size_t n)
{
	uint8_t*	ptr = (uint8_t*)s;
	word		wc = REPEAT_BYTE_IN_WORD(c);

	if (!n)
	{
		return s;
	}

	*(uint8_t*)s = c;
	*((uint8_t*)s + n - 1) = c;

	if (n <= 2)
	{
		return s;
	}

	*(uint16_t*)(s + 1) = (uint16_t)wc;
	*(uint16_t*)(s + n - 3) = (uint16_t)wc;

	if (n <= 6)
	{
		return s;
	}

	*((uint8_t*)s + 3) = c;
	*((uint8_t*)s + n - 4) = c;
	
	if (n <= 8)
	{
		return s;
	}

	// Note: Same as for SS2, aligning pointer.
	uint32_t k = -(uintptr_t)s & 3;
	ptr += k;
	n -= k;
	// Note: Truncate n to a multiple of 4 to not overflow.
	n &= -4;

	*(word*)(ptr) = wc;
	*(word*)(ptr + n - 4) = wc;

	if (n <= 8)
	{
		return s;
	}
	
	*(word*)(ptr + 4) = wc;
	*(word*)(ptr + 8) = wc;
	*(word*)(ptr + n - 8) = wc;
	*(word*)(ptr + n - 12) = wc;

	if (n <= 24)
	{
		return s;
	}

	*(word*)(ptr + 12) = wc;
	*(word*)(ptr + 16) = wc;
	*(word*)(ptr + 20) = wc;
	*(word*)(ptr + 24) = wc;
	*(word*)(ptr + n - 16) = wc;
	*(word*)(ptr + n - 20) = wc;
	*(word*)(ptr + n - 24) = wc;
	*(word*)(ptr + n - 28) = wc;

	k = 24 + ((uintptr_t)ptr & 4);
	ptr += k;
	n -= k;

	do
	{
		*(word*)(ptr) = wc;
		*(word*)(ptr + 4) = wc;
		*(word*)(ptr + 8) = wc;
		*(word*)(ptr + 12) = wc;
		*(word*)(ptr + 16) = wc;
		*(word*)(ptr + 20) = wc;
		*(word*)(ptr + 24) = wc;
		*(word*)(ptr + 28) = wc;

		n -= 32;
		ptr += 32;

	} while (n >= 32);
	
	return s;

}
