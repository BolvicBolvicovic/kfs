#ifndef KARENA_H
#define KARENA_H

#include <c_types.h>
#include <bits.h>
#include <sizes.h>

typedef u32	karena_flags_t;
enum
{
	KARENA_FLAG_NO_CHAIN	= BIT0,
	KARENA_FLAG_LARGE_PAGES	= BIT1,
};

#define KARENA_FLAGS_DEFAULT		0
#define KARENA_COMMIT_SIZE_DEFAULT	KB(1)
#define KARENA_RESERVE_SIZE_DEFAULT	KB(32)
#define KARENA_HEADER_SIZE		128

/* Name: karena_parameters_t
 * Description: parameters to create a karena_t that are passed to karena_alloc.
 * */
typedef struct karena_parameters_t karena_parameters_t;
struct karena_parameters_t
{
	karena_flags_t	flags;
	u32		reserve_size;
	u32		commit_size;
	void*		optional_backbuffer;
	char*		allocation_site_file;
	s32		allocation_site_line;
};

/* Name: karena_t
 * Description: structure that contains the information related to a kernel arena (stack in the heap). 
 * */
typedef struct karena_t karena_t;
struct karena_t
{
	karena_t*	prev;
	karena_t*	current;
	karena_flags_t	flags;
	u32		position;
	u32		base_position;
	u32		reserve_size;
	u32		commit_size;
	u32		reserve;
	u32		commit;
	char*		allocation_site_file;
	s32		allocation_site_line;
};

/* Name: karena_temp_t
 * Description: structure for temporary scope in kernel arenas.
 * */
typedef struct karena_temp_t karena_temp_t;
struct karena_temp_t
{
	karena_t*	arena;
	u32		pos;
};

/* karena_t creation/destruction */
karena_t*	karena_alloc(karena_parameters_t*);
#define KARENA_ALLOC(...) karena_alloc(&(karena_parameters_t){		\
		.flags 			= KARENA_FLAGS_DEFAULT,		\
		.reserve_size 		= KARENA_RESERVE_SIZE_DEFAULT,	\
		.commit_size 		= KARENA_COMMIT_SIZE_DEFAULT,	\
		.allocation_site_file 	= __FILE__,			\
		.allocation_site_line 	= __LINE__,			\
	       	__VA_ARGS__})
void		karena_release(karena_t*);

/* karena_t push/pop/pos core funtions */
void*		karena_push(karena_t*, u32 size, u32 align, bool zero);
u32		karena_pos(karena_t*);
void		karena_pop_to(karena_t*, u32 pos);

/* karena_t push/pop helpers */
void		karena_clear(karena_t*);
void		karena_pop(karena_t*, u32 amount);

/* karena_temp_t scopes */
karena_temp_t	karena_temp_begin(karena_t*);
void		karena_temp_end(karena_temp_t);

/* karena_t push helper macros */
#define KARENA_PUSH_ARRAY_NO_ZERO_ALIGNED(a, T, c, align)	\
	(T *)karena_push((a), sizeof(T)*(c), (align), 0)
#define KARENA_PUSH_ARRAY_ALIGNED(a, T, c, align)		\
	(T *)karena_push((a), sizeof(T)*(c), (align), 1)
#define KARENA_PUSH_ARRAY_NO_ZERO(a, T, c)			\
	KARENA_PUSH_ARRAY_NO_ZERO_ALIGNED(a, T, c, 4)
#define KARENA_PUSH_ARRAY(a, T, c)				\
	KARENA_PUSH_ARRAY_ALIGNED(a, T, c, 4)
#define KARENA_PUSH_STRUCT(a, T)				\
	KARENA_PUSH_ARRAY(a, T, 1)

#endif
