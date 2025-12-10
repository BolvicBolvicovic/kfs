#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <atomic.h>

/* Name: spinlock_t
 * Descrition: classic spinlock that keeps the thread running.
 * Great for small workloads.
 * */
typedef struct
{
	u32	counter;
} spinlock_t;

/* Name: spinlock_lock
 * Descrition: waits until lock can be acquired.
 * Halts if the lock cannot be acquired.
 * */
static inline void
spinlock_lock(spinlock_t* sl)
{
	while (atomic_cmpxchg((atomic_t*)sl, 0, 1))
	{
		// Note: if it was a user friendly implementation, I would use "pause" instead.
		asm volatile ("hlt");
	}
}

/* Name: spinlock_unlock
 * Descrition: do an atomic decrementation to unlock the spinlock.
 * */
static inline void
spinlock_unlock(spinlock_t* sl)
{
	atomic_dec_and_test_zero((atomic_t*)sl);
}

/* Name: spinlock_try_lock
 * Descrition: tries to acquire a lock.
 * On success returns 1 else returns 0.
 * */
static inline bool
spinlock_try_lock(spinlock_t* sl)
{
	if (atomic_cmpxchg((atomic_t*)sl, 0, 1) == 0)
		return 1;
	else
		return 0;
}

/* Name: SPINLOCK_DEFINE
 * Descrition: defines a spinlock with the input name.
 * */
#define SPINLOCK_DEFINE(name)	spinlock_t name = {0}

#endif
