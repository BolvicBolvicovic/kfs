#include "vmm.h"

#define MAX_ALLOC_C_SAME_TIME PAGE_SIZE
#define MAX_ALLOC_B_SAME_TIME PAGE_SIZE / 8
#define MAX_SIZE_ALLOC_SAME_TIME 0xFFFFF
#define MAX_SIZE_B_ALLOC 2048
#define MIN_SIZE_B_ALLOC 8
#define MAX_BITMAP_ELEM 16
#define FREE_LIMIT_B_ALLOC 0xF00

#define CHECK_UPPER_HALF_BITMAP     0xFFFF0000
#define CHECK_REMAINING_HALF_BITMAP 0xFF00FF00
#define CHECK_NEXT_QUARTER_BITMAP   0xF0F0F0F0
#define CHECK_NEXT_EIGHTH_BITMAP    0xCCCCCCCC
#define CHECK_FINAL_PART_BITMAP     0xAAAAAAAA
#define CHECK_LAST_BIT_BITMAP       0x80000000

#define PAGE_ADDR_MASK              0xFFFFF000
#define OFFSET_ADDR_MASK            ~PAGE_ADDR_MASK

typedef struct {
    uint32_t virt_addr;
    uint16_t nb_blocks;
    uint16_t free;
} __attribute__((__packed__)) cont_map_t;

typedef struct {
    uint32_t virt_addr;
    uint16_t size_type : 12;
    uint16_t offset_next_free : 10;
    uint8_t  bitmap_offset: 4;
    uint8_t  bitmap_elem_offset: 6;
    uint32_t bitmap[MAX_BITMAP_ELEM];
} __attribute__((__packed__)) bini_map_t;

static cont_map_t continuous_allocator_map[MAX_ALLOC_C_SAME_TIME] = {0};
static bini_map_t bining_allocator_map[MAX_ALLOC_B_SAME_TIME] = {0};

void*   bining_allocator(size_t size) {
    size_t i;
    size_t size_type;
    for (size_type = 8; size_type < size; size_type <<= 1);
    for (i = 0; i < MAX_ALLOC_B_SAME_TIME; i++) {
        if (!bining_allocator_map[i].virt_addr) break;
        if (bining_allocator_map[i].offset_next_free * bining_allocator_map[i].size_type == PAGE_SIZE) continue;
        if (bining_allocator_map[i].size_type == size_type) {
            uint32_t virt_addr = bining_allocator_map[i].virt_addr + bining_allocator_map[i].offset_next_free * bining_allocator_map[i].size_type;
            bining_allocator_map[i].offset_next_free++;
            bining_allocator_map[i].bitmap[bining_allocator_map[i].bitmap_offset] |= (1 << bining_allocator_map[i].bitmap_elem_offset);
            if (++bining_allocator_map[i].bitmap_elem_offset == 31) {
                bining_allocator_map[i].bitmap_elem_offset = 0;
                bining_allocator_map[i].bitmap_offset++;
            }
            return virt_addr;
        }
    }
    if (i == MAX_ALLOC_B_SAME_TIME) return NULL;
    for (i = 0; i < MAX_ALLOC_B_SAME_TIME; i++) {
        if (!bining_allocator_map[i].size_type) break;
    }
    void* new_page = vmm_alloc_blocks(1);
    if (new_page == NULL) return NULL;
    bining_allocator_map[i].virt_addr = (uint32_t)new_page;
    bining_allocator_map[i].size_type = size_type;
    bining_allocator_map[i].offset_next_free = 1;
    bining_allocator_map[i].bitmap[0] |= 1;
    bining_allocator_map[i].bitmap_elem_offset = 1;
    return new_page;
}

int    bining_allocator_free(uint32_t addr) {
    uint32_t page = addr & PAGE_ADDR_MASK;
    size_t i;
    for (i = 0; i < MAX_ALLOC_B_SAME_TIME; i++) {
        if (!bining_allocator_map[i].virt_addr) break;
        if (bining_allocator_map[i].virt_addr == addr) {
            uint16_t bitmap_index = (addr & OFFSET_ADDR_MASK) / (bining_allocator_map[i].size_type * MAX_BITMAP_ELEM);
            uint8_t bitmap_elem_offset = (addr & OFFSET_ADDR_MASK) % (bining_allocator_map[i].size_type * MAX_BITMAP_ELEM);
            bining_allocator_map[i].bitmap[bitmap_index] &= ~(1 << bitmap_elem_offset);
            if (bining_allocator_map[i].offset_next_free * bining_allocator_map[i].size_type >= FREE_LIMIT_B_ALLOC) {
                uint16_t count = 0;
                for (size_t j = 0; j < MAX_BITMAP_ELEM; j++) {
                    if (!bining_allocator_map[i].bitmap[j]) count++;
                }
                if (count == MAX_BITMAP_ELEM) {
                    vmm_free_blocks(bining_allocator_map[i].virt_addr, 1);
                    // Do not set virt_addr  to 0 as it is used to find the edge of the bining_allocator_map
                    bining_allocator_map[i].size_type = 0;
                    bining_allocator_map[i].offset_next_free = 0;
                    memset(bining_allocator_map[i].bitmap, 0, MAX_BITMAP_ELEM);
                    bining_allocator_map[i].bitmap_elem_offset = 0;
                }
            }
            return 1;
        }
    }
    return 0;
}

void*   kmalloc(size_t size) {
    if (size <= MAX_SIZE_B_ALLOC) return bining_allocator(size);

    uint32_t total_pages_needed = size / PAGE_SIZE + (size % PAGE_SIZE ? 1 : 0);
    size_t i;
    size_t j;
    for (j = 0, i = 0; i < MAX_ALLOC_C_SAME_TIME; i++) {
	    if (!continuous_allocator_map[i].free) {
	        j++;
	        if (!continuous_allocator_map[i].nb_blocks && !continuous_allocator_map[i].virt_addr) break;
	    }
	    if (continuous_allocator_map[i].free && continuous_allocator_map[i].nb_blocks == total_pages_needed) {
	        continuous_allocator_map[i].free = 1;
	        vmm_set_flags_pages(continuous_allocator_map[i].virt_addr, continuous_allocator_map[i].nb_blocks, I86_PTE_WRITABLE, 1);
	        return (void*)continuous_allocator_map[i].virt_addr;
	    }
    }
    if (i == MAX_ALLOC_C_SAME_TIME && j == MAX_ALLOC_C_SAME_TIME) return NULL;
    if (i == MAX_ALLOC_C_SAME_TIME) {
	    for (i = 0; i < MAX_ALLOC_C_SAME_TIME; i++) {
	        if (continuous_allocator_map[i].free) {
	        	vmm_free_blocks(continuous_allocator_map[i].virt_addr, continuous_allocator_map[i].nb_blocks);
	        	break;
	        }
	    }
    }
    void* block_virt_addr = vmm_alloc_blocks(total_pages_needed);
    if (block_virt_addr == NULL) return NULL;
    // vmm_alloc_blocks sets the flags, no need to do it again
    continuous_allocator_map[i].virt_addr = (uint32_t)block_virt_addr;
    continuous_allocator_map[i].nb_blocks = total_pages_needed;
    continuous_allocator_map[i].free = 0;

    return block_virt_addr;
}

void    kfree(void* virt_addr) {
    if (virt_addr == NULL) goto error;
    if (bining_allocator_free((uint32_t)virt_addr)) return;
    for (size_t i = 0; i < MAX_ALLOC_C_SAME_TIME; i++) {
	    if (!continuous_allocator_map[i].free && !continuous_allocator_map[i].nb_blocks && !continuous_allocator_map[i].virt_addr) break;
	    if (continuous_allocator_map[i].virt_addr == (uint32_t)virt_addr) {
	        vmm_set_flags_pages(continuous_allocator_map[i].virt_addr, continuous_allocator_map[i].nb_blocks, I86_PTE_WRITABLE, 0);
	        continuous_allocator_map[i].free = 1;
	        return;
	    }
    }
error:
    printf("ERROR: Invalid free\n");
}

uint32_t kget_size(void* virt_addr) {
    if (virt_addr == NULL) return 0;
    size_t i;
    uint32_t page = (uint32_t)virt_addr & PAGE_ADDR_MASK;
    for (i = 0; i < MAX_ALLOC_B_SAME_TIME; i++) {
        if (!bining_allocator_map[i].virt_addr) break;
        if (bining_allocator_map[i].virt_addr == (uint32_t)virt_addr) {
            return bining_allocator_map[i].size_type;
        }
    }
    for (i = 0; i < MAX_ALLOC_C_SAME_TIME; i++) {
	    if (!continuous_allocator_map[i].free && !continuous_allocator_map[i].nb_blocks && !continuous_allocator_map[i].virt_addr) break;
	    if (continuous_allocator_map[i].virt_addr == (uint32_t)virt_addr) {
	        return continuous_allocator_map[i].nb_blocks * PAGE_SIZE;
	    }
    }
    return 0;
}
