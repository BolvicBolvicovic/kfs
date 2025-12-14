#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <processes/processes.h>
#include "spinlock.h"

/* Name: semaphore_t
 * Descrition: ensures mutual exclusion between two or more processes.
 * */
typedef struct
{
	process_t*	head;
	process_t*	tail;
	spinlock_t	sl_list;
	atomic_t	counter;
} semaphore_t;

/* Name: semaphore_wait
 * Descrition: decrements the value of the semaphore, and if the semaphore is negative,
 * puts the process on the waiting queue until the semaphore is released by the process holding it.
 * */
static inline void
semaphore_wait(semaphore_t* sem)
{
	if (atomic_dec_and_test_neg(&sem->counter))
	{
		spinlock_lock(&sem->sl_list);
		scheduler_lock();

		process_t*	current_process = current_process_pop();

		if (!sem->head)
		{
			sem->head = current_process;
		}
		else
		{
			sem->tail->next = current_process;
		}

		sem->tail = current_process;

		spinlock_unlock(&sem->sl_list);
		scheduler_unlock();

		// Note: trigger the scheduler
		asm volatile ("int $0x20");
	}
}

/* Name: semaphore_signal
 * Descrition: increments the semaphore and, if it is still negative,
 * indicates to the scheduler to wake the next waiting process in the queue.
 * */
static inline void
semaphore_signal(semaphore_t* sem)
{
	if (atomic_inc_and_test_neg(&sem->counter))
	{
		spinlock_lock(&sem->sl_list);

		new_process_list_push(sem->head);
		sem->head = (process_t*)sem->head->next;

		spinlock_unlock(&sem->sl_list);
	}
}

/* Name: SEMAPHORE_DEFINE
 * Descrition: defines a semaphore_t with input name and sets its atomic counter to 0.
 * */
#define	SEMAPHORE_DEFINE(name)		semaphore_t	name = {0,0,{0},{0}}

#endif
