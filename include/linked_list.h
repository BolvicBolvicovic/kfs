#ifndef LINKED_LIST_H
#define LINKED_LIST_H

/* Name: single_ll_t
 * Description: singly linked list node definition.
 * */
typedef struct single_ll_node_t single_ll_node_t;
struct single_ll_node_t
{
	void*			data;
	single_ll_node_t*	next;
};

#define SLL_STACK_PUSH_N(f, n, next)	((n)->next=(f), (f)=(n))
#define SLL_STACK_POP_N(f, next)	((f)=(f)->next)

/* Name: single_ll_push
 * Description: push a node on top of a list.
 * */
static inline void
single_ll_push(single_ll_node_t** head, single_ll_node_t* node)
{
	SLL_STACK_PUSH_N(*head, node, next);
}

/* Name: single_ll_pop
 * Description: pops head node from a list and returns it.
 * */
static inline single_ll_node_t*
single_ll_pop(single_ll_node_t** head)
{
	single_ll_node_t*	res = *head;

	SLL_STACK_POP_N(*head, next);

	return res;
}

#define LINKED_LIST_STRUCT(name, T)	\
typedef struct name name;		\
struct name				\
{					\
	T*	head;			\
	T*	tail;			\
}

#endif
