#include "pmm.h"

static uint32_t  _memory_size           = 0;
static uint32_t  _memory_used_blocks    = 0;
static uint32_t  _memory_max_blocks     = 0;
static uint32_t* _memory_map            = NULL;

inline void mmap_set(int bit) {
    _memory_map[bit / 32] |= (1 << (bit % 32));
}

inline void mmap_unset(int bit) {
    _memory_map[bit / 32] &= ~(1 << (bit % 32));
}

inline int mmap_test(int bit) {
    return _memory_map[bit / 32] & (1 << (bit % 32));
} 

int mmap_find_first_free() {
    for (size_t i = 0; i < _memory_max_blocks / 32; i++) {
        if (_memory_map[i] != 0xFFFFFFFF) {
            for (size_t j = 0; j < 32; i++) {
                int bit = 1 << j;
                if (!(_memory_map[i] & bit)) {
                    return i * 32 + j;
                }
            }
        }
    }
}

void pmm_init(size_t mem_size, uint32_t bitmap) {
    _memory_size = mem_size;
    _memory_map  = (uint32_t*)bitmap;
    _memory_max_blocks = _memory_size * 1024 / PMM_BLOCK_SIZE;
    _memory_used_blocks = _memory_max_blocks;

    memset(_memory_map, 0xF, _memory_max_blocks / PMM_BLOCKS_PER_BYTE);
}

void pmm_init_region(uint32_t base, size_t size) {
    int align = base / PMM_BLOCK_SIZE;
    int blocks = size / PMM_BLOCK_SIZE;
    for (; blocks > 0; blocks--) {
        mmap_unset(align++);
        _memory_used_blocks--;
    }
    mmap_set(0);
}

void pmm_deinit_region(uint32_t base, size_t size) {
    int align = base / PMM_BLOCK_SIZE;
    int blocks = size / PMM_BLOCK_SIZE;
    for (; blocks > 0; blocks--) {
        mmap_set(align++);
        _memory_used_blocks++;
    }
}

void* pmm_allock_block() {
    if (_memory_max_blocks - _memory_used_blocks <= 0) return NULL;
    int frame = mmap_find_first_free();
    if (frame == -1) return NULL;
    mmap_set(frame);
    uint32_t addr = frame * PMM_BLOCK_SIZE;
    _memory_used_blocks++;
    return (void*)addr;
}

void pmm_free_block(void* p) {
    uint32_t addr = (uint32_t)p;
    int frame = addr / PMM_BLOCK_SIZE;
    mmap_unset(frame);
    _memory_used_blocks--;
}
