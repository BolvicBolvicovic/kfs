#include "pmm.h"

uint32_t  _memory_size           = 0;
uint32_t  _memory_used_blocks    = 0;
uint32_t  _memory_max_blocks     = 0;
uint32_t* _memory_map            = NULL;

void mmap_set(int bit) {
    _memory_map[bit / 32] |= (1 << (bit % 32));
}

void mmap_unset(int bit) {
    _memory_map[bit / 32] &= ~(1 << (bit % 32));
}

int mmap_test(int bit) {
    return _memory_map[bit / 32] & (1 << (bit % 32));
} 

int mmap_find_first_free() {
    for (size_t i = 0; i < _memory_max_blocks - _memory_used_blocks; i++) {
        if (_memory_map[i] != 0xFFFFFFFF) {
            for (size_t j = 0; j < 32; j++) {
                uint32_t bit = 1 << j;
                if (!(_memory_map[i] & bit)) {
                    return i * 32 + j;
                }
            }
        }
    }
    return (-1);
}

int mmap_find_first_free_s (size_t size) {

	if (size==0)
		return -1;

	if (size==1)
		return mmap_find_first_free ();

	for (size_t i = 0; i < _memory_max_blocks - _memory_used_blocks; i++)
		if (_memory_map[i] != 0xffffffff)
			for (uint32_t j=0; j<32; j++) {	//! test each bit in the dword

				uint32_t bit = 1<<j;
				if (! (_memory_map[i] & bit) ) {

					uint32_t startingBit = i*32 + j;

					uint32_t free=0; //loop through each bit to see if its enough space
					for (uint32_t count=0; count<=size;count++) {

						if (! mmap_test (startingBit+count) )
							free++;	// this bit is clear (free frame)

						if (free==size)
							return startingBit; //free count==size needed; return index
					}
				}
			}

	return -1;
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

void* pmm_alloc_block() {
    if (_memory_max_blocks - _memory_used_blocks <= 0) return NULL;
    int frame = mmap_find_first_free();
    if (frame == -1) return NULL;
    mmap_set(frame);
    uint32_t addr = frame * PMM_BLOCK_SIZE;
    _memory_used_blocks++;
    printf("new pmm alloc : %p\n", (void *)addr);
    return (void*)addr;
}

void* pmm_alloc_blocks(size_t nb_blocks) {
    if (_memory_max_blocks - _memory_used_blocks <= 0) return NULL;
    int frame = mmap_find_first_free_s(nb_blocks);
    if (frame == -1) return 0;
    for (size_t i = 0; i < nb_blocks; i++) mmap_set(frame + i);
    uint32_t addr = frame * PMM_BLOCK_SIZE;
    _memory_used_blocks += nb_blocks;
    printf("new pmm alloc+: %p\n", (void *)addr);
    return (void*)addr;
}

void pmm_free_block(void* p) {
    uint32_t addr = (uint32_t)p;
    int frame = addr / PMM_BLOCK_SIZE;
    mmap_unset(frame);
    _memory_used_blocks--;
}

void	pmm_free_blocks (void* p, size_t size) {
	uint32_t addr = (uint32_t)p;
	int frame = addr / PMM_BLOCK_SIZE;

	for (uint32_t i = 0; i < size; i++)
		mmap_unset (frame + i);

	_memory_used_blocks -= size;
}
