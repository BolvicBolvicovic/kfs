#include "stdlib.h"

#define HEX_STR	"0123456789ABCDEF"

inline s32
isnum(const char c)
{
	return c >= 0x30 && c <= 0x39;
}

static inline u32
nb_size(s32 nb)
{
	u32	i = 1;

	for (; nb > 9; i++) nb /= 10;

	return i;
}

static inline u32
unb_size(u32 nb)
{
	u32	i = 1;
	
	for (; nb >= 10; i++) nb /= 10;
	
	return i;
}

u32
itoa(char* dest, s32 nb)
{
	u32	j	= 0;
	bool	neg	= 0;

	if (nb < 0)
	{
		dest[j]	= '-';
		neg	= 1;
		nb	= -nb;
		j++;
	}

	j	+= nb_size(nb);
	dest[j]	= 0;

	for (s32 i = j; i-- > 0;)
	{
		if (!i && neg) break;

		dest[i] = nb % 0xA + 0x30;
		nb /= 0xA;
	}

	return j;
}

u32
utoa(char* dest, u32 nb)
{
	u32	j = 0;

	j	+= unb_size(nb);
	dest[j]	= '\0';

	for (s32 i = j; i-- > 0;)
	{
		dest[i]	= nb % 10 + '0';
		nb	/= 10;
	}

	return j;
}

u32
itox(char* dest, u32 nb)
{
	char	hex[]	= HEX_STR;

	for (char* itr = dest + 7; itr >= dest; itr--)
	{
		*itr	= hex[nb % 0x10];
		nb	/= 0x10;
	}

	dest[8] = 0;

	return 8;
}

u32
_itoxx(char *dest, u32 n)
{
	char	hex[] = HEX_STR;

	*dest	= hex[n % 0x10];
	n	/= 0x10;

	if (n != 0) return (1 + _itoxx(dest - 1, n));

	return (1);
}

u32
itoxx(char *dest, u32 n)
{
	u32	size = 0;
	u32	nb = n;

	while (nb)
	{
		nb /= 0x10;
		++size;
	}

	dest[size] = '\0';

	return (_itoxx(dest + size - 1, n));
}

inline s32
atoi(const char *nptr)
{
	if (!nptr) return 0;

	s32	sign	= 1;
	s32	res	= 0;

	if (*nptr == '+' || *nptr == '-')
	{
		if (*(nptr) == '-') sign = -1;
		nptr++;
	}

	while (*nptr && isnum(*nptr))
	{
		res = res * 10 + *nptr - 0x30;
		nptr++;
	}

	return res * sign;
}
