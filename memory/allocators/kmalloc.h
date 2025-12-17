#ifndef KMALLOC_H
#define KMALLOC_H

#include <c_types.h>

/* Name: kmalloc
 * Description: generic allocator that combines a bining allocator with variable granularity
 * and continuous allocator that has a 4KB granularity.
 * Use ONLY if you need absolute controle on the lifetime of a SINGLE object. */
void*   kmalloc(u32 size);

/* Name: kfree
 * Description: counter part to kmalloc.
 * Does not free virtual memory. It is recycled by the allocator.
 * */
void    kfree(void* virt_addr);

/* Name: kget_size
 * Description: returns the actual size of the allocated block for a virtual address.
 * */
u32	kget_size(void* virt_addr);


#endif
