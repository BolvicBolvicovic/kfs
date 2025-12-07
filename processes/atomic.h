#ifndef ATOMIC_H
#define ATOMIC_H

#include "c_types.h"
#include "compiler.h"

typedef struct
{
	u32	counter;
} atomic_t;

/* Name: __READ_ONCE
 * Descrition: atomic read once if sizeof(x) == 4 else simple read once
 * */
#define __READ_ONCE(x)	(*(volatile typeof(x)*)&(x))

/* Name: atomic_read
 * Descrition: type and atomic safe version of __READ_ONCE.
 * */
static __always_inline u32
atomic_read(atomic_t* x)
{
	return __READ_ONCE(x->counter);
}

/* Name: __WRITE_ONCE
 * Descrition: atomic write once if sizeof(x) == 4 else simple write once
 * */
#define __WRITE_ONCE(x, val)	\
do				\
{				\
	__READ_ONCE(x) = (val);	\
} while (0);

/* Name: atomic_write
 * Descrition: type and atomic safe version of __WRITE_ONCE.
 * */
static __always_inline void
atomic_write(atomic_t* x, u32 val)
{
	__WRITE_ONCE(x->counter, val);
}

/* Name: atomic_inc
 * Descrition: atomic incrementation of x->counter
 * */
static __always_inline void
atomic_inc(atomic_t* x)
{
	asm volatile ("lock incl %0" : "+m"(x->counter));
}

/* Name: atomic_dec_and_test
 * Descrition: atomic decrementation of x->counter and return 1 if x->counter == 0 else 0.
 * */
static __always_inline bool
atomic_dec_and_test(atomic_t* x)
{
	bool	is_zero;

	asm volatile
	(
		"lock decl %0\n\t"
		"sete %1"
		: "+m"(x->counter), "=qm"(is_zero)
	);

	return is_zero;
}

/* Name: atomic_cmpxchg
 * Descrition: atomic compare and exchange. 
 * Compare old with x->counter, if identical, store new in x->counter.
 * Return the initial value in x->counter.
 * Success is indicated by comparing returned value with old.
 * */
static __always_inline u32
atomic_cmpxchg(atomic_t* x, u32 old, u32 new)
{
	u32	ret;

	asm volatile
	(
		"lock cmpxchgl %2, %1" 
		: "=a"(ret), "+m"(x->counter)
		: "r"(new), "0"(old)
		: "memory"
	);

	return ret;
}

#endif
