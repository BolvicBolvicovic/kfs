#ifndef SLICE_H
#define SLICE_H

#include <c_types.h>

typedef struct slice_t slice_t;
struct slice_t
{
	u32	start;	
	u32	end;
};

typedef struct sub_array_t sub_array_t;
struct sub_array_t
{
	void*	data;
	slice_t	slice;
};

#endif
