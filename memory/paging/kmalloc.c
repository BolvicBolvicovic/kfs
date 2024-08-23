#include "paging.h"

extern uint8_t kheap_start;

uint32_t placement_addr;

void init_kmalloc() { placement_addr = (uint32_t)&kheap_start; }

uint32_t kmalloc(size_t size, int align, uint32_t* phys_addr) {
	if (align && (placement_addr & 0x00000FFF)) { // If addr not already page-aligned, align it
		placement_addr &= 0x00000FFF;
		placement_addr += 0x1000; // 4K paging
	}
	if (phys_addr) {
		*phys_addr = placement_addr;
	}
	uint32_t tmp = placement_addr;
	placement_addr += size;
	return tmp;
}
