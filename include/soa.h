/* Name: soa.h
 * Description: header file containing helper macro for Structure Of Arrays (SOA).
 * SOAs are part of the data-oriented way of programming and are to put in opposition against 
 * Arrays Of Structures (AOEs) that come from the traditional OOP.
 * * * * * *
 * typedef struct my_struct my_struct;
 * struct my_struct
 * {
 *	u32	var0;
 *	u32	var1;
 *
 *	// other fields...
 * };
 *
 * // AOS
 * typedef my_struct* aos_my_struct;
 *
 * // SOA
 * typedef struct my_struct my_struct;
 * struct my_struct
 * {
 *	u32*	var0;
 *	u32*	var1;
 *
 *	// other array fields...
 * };
 * * * * * *
 * SOA seems a bit more cumbersome to use in C but has many architectural advantages over AOS, namely:
 * 	- SIMD friendly
 * 	- cache friendly
 * 	- parallelization friendly
 * Because we want performance, SOA is the ideal design choice.
 * * * * * *
 * Written by: Victor Cornille
 * */

#ifndef SOA_H
#define SOA_H

#include <memory/allocators/karena.h>
#include <memory/allocators/kmalloc.h>

/* Name: SOA_DEFINE_FIELD
 * Description: X helper that defines name of type.
 * */
#define SOA_DEFINE_FIELD(type, name)		type name;

/* Name: SOA_DEFINE_FIELD_ARRAY
 * Description: X helper that defines an array name of type.
 * */
#define SOA_DEFINE_FIELD_ARRAY(type, name)	type * name;

/* Name: SOA_DEFINE_STRUCT_OF_ARRAYS
 * Description: X-macro that defines a structure of arrays for a structure.
 * The structure of array will be named soa_##type.
 * * * * * * 
 * @type	: struct name
 * @FIELDS	: X-macro that defines fields and takes as parameter either SOA_DEFINE_FIELD_ARRAY,
 * SOA_DEFINE_FIELD, SOA_ALLOC_KARENA_FIELD_ARRAY or SOA_ALLOC_KMALLOC_FIELD_ARRAY.
 * The X macro used in FIELDS has 2 fields: type and name.
 * * * * * * 
 * When creating the X-macro FIELDS, remember to not write the ';' of the last field.
 * When using this macro, remember finishing your line with a ';'.
 * * * * * * 
 * Usage
 * * * * * * 
 * // my_struct.h
 *
 * typedef struct my_struct my_struct;
 * struct my_struct
 * {
 * #define MY_STRUCT_FIELDS(X)	\
 * 	X(int,		var1);	\
 * 	X(float,	var2)
 *
 * 	MY_STRUCT_FIELDS(SOA_ALLOC_FIELD);
 * };
 *
 * SOA_DEFINE_STRUCT_OF_ARRAYS(my_struct, MY_STRUCT_FIELDS);
 * * * * * *
 * // my_struct.c
 *
 * static void
 * my_func(soa_my_struct* soa, u32 size)
 * {
 * 	for (u32 i = 0; i < size; i++)
 * 	{
 * 	     soa->var1[i] = do_something(soa->var2[i])
 * 	}
 * }
 * */
#define SOA_DEFINE_STRUCT_OF_ARRAYS(type, FIELDS)	\
typedef struct soa_##type soa_##type;			\
struct soa_##type					\
{							\
	FIELDS(SOA_DEFINE_FIELD_ARRAY)			\
}

/* Name: SOA_ALLOC_KARENA_FIELD_ARRAY
 * Description: helper macro X used with FIELDS that pushes a field array onto an arena.
 * Do not use it in your code.
 * */
#define SOA_ALLOC_KARENA_FIELD_ARRAY(type, name)	\
	(SOA)->name = KARENA_PUSH_ARRAY((SOA_ARENA), type, (SOA_SIZE));

/* Name: SOA_ALLOC_KARENA_STRUCT_OF_ARRAYS
 * Description: pushes SOA onto SOA_ARENA and all its field arrays.
 * Mainly useful to write a special function that allocates a structure of arrays for a user defined
 * structure that DOES NOT own its lifetime.
 * * * * * * 
 * @soa_type	: type of the soa.
 * @FIELDS	: X-macro that defines fields and takes as parameter either SOA_DEFINE_FIELD_ARRAY,
 * SOA_DEFINE_FIELD, SOA_ALLOC_KARENA_FIELD_ARRAY or SOA_ALLOC_KMALLOC_FIELD_ARRAY.
 * The X macro used in FIELDS has 2 fields: type and name.
 * External Macro Parameters:
 * #SOA		: defines which soa_my_struct pointer will receive the newly allocated soa.
 * #SOA_ARENA	: defines which arena we can push SOA and its field arrays onto.
 * #SOA_SIZE	: defines the size for each field arrays.
 * * * * * * 
 * Remember to #undef each external macro parameters after their usage
 * to avoid polluting the global name space.
 * When using this macro, remember finishing your line with a ';'.
 * * * * * * 
 * Usage
 * * * * * * 
 * static soa_my_struct*
 * my_struct_alloc_karena_soa(karena_t* arena, u32 size)
 * {
 *	soa_my_struct*	my_soa;
 *
 * #define SOA		my_soa
 * #define SOA_ARENA	arena
 * #define SOA_SIZE	size
 *
 *	SOA_ALLOC_KARENA_STRUCT_OF_ARRAYS(soa_my_struct, MY_STRUCT_FIELDS);
 *
 * #undef SOA
 * #undef SOA_ARENA
 * #undef SOA_SIZE
 *
 * 	// You can do extra checks if you want to see if an allocation failed
 *	return my_soa;
 * }
 * */
#define SOA_ALLOC_KARENA_STRUCT_OF_ARRAYS(soa_type, FIELDS)	\
do {								\
	(SOA) = KARENA_PUSH_STRUCT(SOA_ARENA, soa_type);	\
	FIELDS(SOA_ALLOC_KARENA_FIELD_ARRAY)			\
} while (0)

/* Name: SOA_ALLOC_KMALLOC_FIELD_ARRAY
 * Description: helper macro X used with FIELDS that pushes a field array onto an arena.
 * Do not use it in your code.
 * */
#define SOA_ALLOC_KMALLOC_FIELD_ARRAY(type, name)	\
	(SOA)->name = (type*)kmalloc(sizeof(type) * (SOA_SIZE));

/* Name: SOA_ALLOC_KMALLOC_STRUCT_OF_ARRAYS
 * Description: allocates and attributes memory for SOA and all its field arrays.
 * Mainly useful to write a special function that allocates a structure of arrays for a user defined
 * structure that DOES own its lifetime.
 * * * * * * 
 * @soa_type	: type of the soa.
 * @FIELDS	: X-macro that defines fields and takes as parameter either SOA_DEFINE_FIELD_ARRAY,
 * SOA_DEFINE_FIELD, SOA_ALLOC_KARENA_FIELD_ARRAY or SOA_ALLOC_KMALLOC_FIELD_ARRAY.
 * The X macro used in FIELDS has 2 fields: type and name.
 * External Macro Parameters:
 * #SOA		: defines which soa_my_struct pointer will receive the newly allocated soa.
 * #SOA_SIZE	: defines the size for each field arrays.
 * * * * * * 
 * Remember to #undef each external macro parameters after their usage
 * to avoid polluting the global name space.
 * When using this macro, remember finishing your line with a ';'.
 * * * * * * 
 * Usage
 * * * * * * 
 * static soa_my_struct*
 * my_struct_alloc_kmalloc_soa(u32 size)
 * {
 *	soa_my_struct*	my_soa;
 *
 * #define SOA		my_soa
 * #define SOA_SIZE	size
 *
 *	SOA_ALLOC_KMALLOC_STRUCT_OF_ARRAYS(soa_my_struct, MY_STRUCT_FIELDS);
 *
 * #undef SOA
 * #undef SOA_SIZE
 *
 * 	// You can do extra checks if you want to see if an allocation failed
 *	return my_soa;
 * }
 * */
#define SOA_ALLOC_KMALLOC_STRUCT_OF_ARRAYS(soa_type, FIELDS)	\
do {								\
	(SOA) = (soa_type*)kmalloc(sizeof(soa_type));		\
	FIELDS(SOA_ALLOC_KMALLOC_FIELD_ARRAY)			\
} while (0)

#endif
