#include "vmm.h"

void*   kmalloc(size_t size) {
    void* block_virt_addr = vmm_alloc_blocks(size);


    return block_virt_addr;
}

void    kfree(uint32_t virt_addr) {
    vmm_free_block(virt_addr);
}
