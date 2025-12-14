#ifndef LINKED_LIST_H
#define LINKED_LIST_H

/* Name: single_ll_t
 * Description: singly linked list node definition.
 * */
typedef struct single_ll_s
{
	void*			data;
	struct single_ll_s*	next;
} single_ll_t;

#define SLL_STACK_PUSH_N(f, n, next)	((n)->next=(f), (f)=(n))
#define SLL_STACK_POP_N(f, next)	((f)=(f)->next)

/* Name: single_ll_push
 * Description: push a node on top of a list.
 * */
static inline void
single_ll_push(single_ll_t** list, single_ll_t* node)
{
	SLL_STACK_PUSH_N(*list, node, next);
}

/* Name: single_ll_pop
 * Description: pops head node from a list and returns it.
 * */
static inline single_ll_t*
single_ll_pop(single_ll_t** list)
{
	single_ll_t*	head = *list;

	SLL_STACK_POP_N(*list, next);

	return head;
}

//typedef struct double_ll_s
//{
//	void*			data;
//	struct double_ll_s*	next;
//	struct double_ll_s*	prev;
//} double_ll_t;

#endif
