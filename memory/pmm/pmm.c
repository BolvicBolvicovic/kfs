#include "pmm.h"

static uint32_t  _memory_size           = 0;
static uint32_t  _memory_used_blocks    = 0;
static uint32_t  _memory_max_blocks     = 0;
static uint32_t* _memory_memory_map     = NULL;

inline void mmap_set(int bit) {
    _memory_memory_map[bit / 32] |= (1 << (bit % 32));
}

inline void mmap_unset(int bit) {
    _memory_memory_map[bit / 32] &= ~(1 << (bit % 32));
}

inline int mmap_test(int bit) {
    return _memory_memory_map[bit / 32] & (1 << (bit % 32));
} 

int mmap_find_first_free() {
    for (size_t i = 0; i < (size_t))
}
