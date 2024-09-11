#include "vmm.h"

#define MAX_ALLOC_SAME_TIME 0x1000

typedef struct {
    uint32_t virt_addr;
    uint32_t nb_blocks : 31;
    uint16_t free : 1;
} __attribute__((__packed__)) freemap_t;

static freemap_t memory_map[MAX_ALLOC_SAME_TIME] = {0};

void*   kmalloc(size_t size) {
    uint32_t total_pages_needed = size / PAGE_SIZE + (size % PAGE_SIZE ? 1 : 0);
    size_t i;
    size_t j;
    for (j = 0, i = 0; i < MAX_ALLOC_SAME_TIME; i++) {
	if (!memory_map[i].free) {
	    j++;
	    if (!memory_map[i].nb_blocks && !memory_map[i].virt_addr) break;
	}
	if (memory_map[i].free && memory_map[i].nb_blocks == total_pages_needed) {
	    memory_map[i].free = 1;
	    vmm_set_flags_pages(memory_map[i].virt_addr, memory_map[i].nb_blocks, I86_PTE_WRITABLE, 1);
	    return (void*)memory_map[i].virt_addr;
	}
    }
    if (i == MAX_ALLOC_SAME_TIME && j == MAX_ALLOC_SAME_TIME) return NULL;
    void* block_virt_addr = vmm_alloc_blocks(total_pages_needed);
    if (i == MAX_ALLOC_SAME_TIME) {
	for (i = 0; i < MAX_ALLOC_SAME_TIME; i++) {
	    if (memory_map[i].free) {
		vmm_free_blocks(memory_map[i].virt_addr, memory_map[i].nb_blocks);
		break;
	    }
	}
    }
    // vmm_alloc_blocks sets the flags, no need to do it again
    memory_map[i].virt_addr = (uint32_t)block_virt_addr;
    memory_map[i].nb_blocks = total_pages_needed;
    memory_map[i].free = 0;

    return block_virt_addr;
}

void    kfree(void* virt_addr) {
    for (size_t i = 0; i < MAX_ALLOC_SAME_TIME; i++) {
	if (!memory_map[i].free && !memory_map[i].nb_blocks && !memory_map[i].virt_addr) break;
	if (memory_map[i].virt_addr == (uint32_t)virt_addr) {
	    vmm_set_flags_pages(memory_map[i].virt_addr, memory_map[i].nb_blocks, I86_PTE_WRITABLE, 0);
	    memory_map[i].free = 1;
	    return;
	}
    }
    printf("ERROR: Invalid free\n");
}

uint32_t kget_size(void* virt_addr) {
    for (size_t i = 0; i < MAX_ALLOC_SAME_TIME; i++) {
	if (!memory_map[i].free && !memory_map[i].nb_blocks && !memory_map[i].virt_addr) break;
	if (memory_map[i].virt_addr == (uint32_t)virt_addr) {
	    return memory_map[i].nb_blocks * PAGE_SIZE;
	}
    }
    return 0;
}
