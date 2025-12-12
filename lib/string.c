#include <string.h>

__always_inline u32
strlen(const char* s)
{
	u32	i = 0;
	
	for (;s[i];i++);

	return i;
}

__always_inline	s32
strcmp(const char* s1, const char* s2)
{
	for (;*s1 && *s2 && *s1 == *s2; s1++, s2++);

	return *s1 - *s2;
}

__always_inline char*
strchr(const char* s, int c)
{
    for (;*s; s++) if (*s == c) return (char*)s;

    return 0;
}

__always_inline char*
strcpy(char* restrict dest, const char* restrict src)
{
	u32   i = 0;

	for (;src[i];i++) dest[i] = src[i];

	dest[i] = 0;

	return dest;
}

inline void*
memcpy(void* dst, const void* src, u32 n)
{
	unsigned char		*d = dst;
	const unsigned char	*s = src;

	uint32_t	w, x;

	for (; (uintptr_t)s % 4 && n; n--) *d++ = *s++;

	if ((uintptr_t)d % 4 == 0)
	{
		for (; n>=16; s+=16, d+=16, n-=16)
		{
			*(uint32_t*)(d+0) = *(uint32_t*)(s+0);
			*(uint32_t*)(d+4) = *(uint32_t*)(s+4);
			*(uint32_t*)(d+8) = *(uint32_t*)(s+8);
			*(uint32_t*)(d+12) = *(uint32_t*)(s+12);
		}

		if (n&8)
		{
			*(uint32_t*)(d+0) = *(uint32_t*)(s+0);
			*(uint32_t*)(d+4) = *(uint32_t*)(s+4);
			d += 8; s += 8;
		}

		if (n&4)
		{
			*(uint32_t*)(d+0) = *(uint32_t*)(s+0);
			d += 4; s += 4;
		}

		if (n&2)
		{
			*d++ = *s++; *d++ = *s++;
		}

		if (n&1)
		{
			*d = *s;
		}

		return dst;
	}
	
	if (n >= 32)
	{
		switch ((uintptr_t)d % 4)
		{
		case 1:
			w = *(uint32_t*)s;
			*d++ = *s++;
			*d++ = *s++;
			*d++ = *s++;
			n -= 3;
			for (; n>=17; s+=16, d+=16, n-=16) {
				x = *(uint32_t*)(s+1);
				*(uint32_t*)(d+0) = (w >> 24) | (x << 8);
				w = *(uint32_t*)(s+5);
				*(uint32_t*)(d+4) = (x >> 24) | (w << 8);
				x = *(uint32_t*)(s+9);
				*(uint32_t*)(d+8) = (w >> 24) | (x << 8);
				w = *(uint32_t*)(s+13);
				*(uint32_t*)(d+12) = (x >> 24) | (w << 8);
			}
			break;
		case 2:
			w = *(uint32_t*)s;
			*d++ = *s++;
			*d++ = *s++;
			n -= 2;
			for (; n>=18; s+=16, d+=16, n-=16) {
				x = *(uint32_t*)(s+2);
				*(uint32_t*)(d+0) = (w >> 16) | (x << 16);
				w = *(uint32_t*)(s+6);
				*(uint32_t*)(d+4) = (x >> 16) | (w << 16);
				x = *(uint32_t*)(s+10);
				*(uint32_t*)(d+8) = (w >> 16) | (x << 16);
				w = *(uint32_t*)(s+14);
				*(uint32_t*)(d+12) = (x >> 16) | (w << 16);
			}
			break;
		case 3:
			w = *(uint32_t*)s;
			*d++ = *s++;
			n -= 1;
			for (; n>=19; s+=16, d+=16, n-=16) {
				x = *(uint32_t*)(s+3);
				*(uint32_t*)(d+0) = (w >> 8) | (x << 24);
				w = *(uint32_t*)(s+7);
				*(uint32_t*)(d+4) = (x >> 8) | (w << 24);
				x = *(uint32_t*)(s+11);
				*(uint32_t*)(d+8) = (w >> 8) | (x << 24);
				w = *(uint32_t*)(s+15);
				*(uint32_t*)(d+12) = (x >> 8) | (w << 24);
			}
			break;
		}
	}

	if (n&16)
	{
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
	}

	if (n&8)
	{
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
	}

	if (n&4)
	{
		*d++ = *s++; *d++ = *s++; *d++ = *s++; *d++ = *s++;
	}

	if (n&2)
	{
		*d++ = *s++; *d++ = *s++;
	}

	if (n&1)
	{
		*d = *s;
	}

	return dst;
}

void*
memset(void* s, uint8_t c, u32 n)
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
