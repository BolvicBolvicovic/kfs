#ifndef C_TYPES_H
#define C_TYPES_H

#include <stdint.h>

// UNSIGNED TYPE
typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;

// SIGNED TYPE
typedef int8_t		s8;
typedef int16_t		s16;
typedef int32_t		s32;

typedef _Bool		bool;

#ifndef UCHAR_MAX
#define UCHAR_MAX	255
#endif

#endif
