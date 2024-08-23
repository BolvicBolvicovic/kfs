#include "paging.h"

uint32_t *frames; // Bitmap - used or free
uint32_t nframes;

extern uint32_t placement_addr;
static page_directory_t* kernel_dir;
static page_directory_t* current_dir;

#define INDEX_FROM_BIT(a) (a/32)
#define OFFSET_FROM_BIT(a) (a%32)

static void set_frame(uint32_t frame_addr) {
	uint32_t frame = frame_addr/0x1000;
	uint32_t idx = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);
	frames[idx] |= (1 << off);
}

static void clear_frame(uint32_t frame_addr) {
	uint32_t frame = frame_addr/0x1000;
	uint32_t idx = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);
	frames[idx] &= ~(1 << off);
}

static uint32_t test_frame(uint32_t frame_addr) {
	uint32_t frame = frame_addr/0x1000;
	uint32_t idx = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);
	return frames[idx] & (1 << off);
}

static uint32_t first_frame() {
	for (size_t i = 0; i < INDEX_FROM_BIT(nframes); i++) {
		if (frames[i] != 0xFFFFFFFF) {
			for (size_t j = 0; j < 32; j++) {
				uint32_t tester = 1 << j;
				if (!(frames[i] & tester)) {
					return i * 32 + j;
				}
			}
		}
	}
	return 0;
}

void alloc_frame(page_t* page, int is_kernel, int is_writeable) {
	if (page->frame != 0) return;
	uint32_t idx = first_frame();
	if (idx == 0xFFFFFFFF) {
		PANIC("No free frames");
	}
	set_frame(idx * 0x1000);
	page->present = 1;
	page->rw = is_writeable ? 1 : 0;
	page->user = is_kernel ? 0 : 1;
	page->frame = idx;
}

void free_frame(page_t* page) {
	uint32_t frame = page->frame;
	if (!frame) return;
	clear_frame(frame);
	page->frame = 0;
}

void initialise_paging() {
	uint32_t mem_end_page = 0x10000000;
	nframes = mem_end_page / 0x1000;
	memset(frames, 0, INDEX_FROM_BIT(nframes));
	kernel_dir = (page_directory_t*)kmalloc(sizeof(page_directory_t), 1, NULL);
	current_dir = kernel_dir;
	for (size_t i = 0; i < placement_addr; i += 0x1000) {
		alloc_frame(get_page(i, 1, kernel_dir), 0, 0);
	}
	register_interrupt_handler(14, page_fault);
	switch_page_dir(kernel_dir);
}

void switch_page_dir(page_directory_t *new_dir) {

}
