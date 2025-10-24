#include "gpu.h"

static uint32_t					gpu_width	= GPU_FALLBACK_WIDTH;
static uint32_t					gpu_height	= GPU_FALLBACK_HEIGHT;
static uint32_t					gpu_display	= GPU_FALLBACK_DISPLAY;
static uint8_t					gpu_bus 	= 0;
static uint8_t					gpu_device	= 0;
static volatile gpu_mmio*		mmio		= 0;
static volatile gpu_ctrl_queue*	ctrl_queue	= 0;
#define GPU_QUEUE_BITMAP_SIZE	(GPU_RING_DESC_CRTLQ_NB / 2)
static uint8_t					ctrl_queue_bitmap[GPU_QUEUE_BITMAP_SIZE] = {0};
static uint8_t					ctrl_queue_bitmap_free = GPU_QUEUE_BITMAP_SIZE;

static const uint32_t			mouse_bitmap[64 * 64] =
{
    // 0–7: empty (transparent)
    0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
    0x00000000,0xFF000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,0x00000000,
    0x00000000,0xFF000000,0xFF000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,
    0x00000000,0xFF000000,0xFF000000,0xFF000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,
    0x00000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFFFFFFFF,0xFFFFFFFF,0x00000000,
    0x00000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFFFFFFFF,0x00000000,
    0x00000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0x00000000,
    0x00000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0x00000000,

    // Middle diagonal and shadow
    0x00000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,
    0x00000000,0xFF000000,0xFF000000,0xFFFFFFFF,0xFFFFFFFF,0xFF000000,0xFF000000,0xFF000000,
    0x00000000,0xFF000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFF000000,0xFF000000,
    0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,

    // body continues roughly centered
    0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,
    0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,
    0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,
    0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,

    // taper end
    0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,
    0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,
    0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,
    0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,

    // fade to transparent toward bottom
    0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,
    0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,
    0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,
    0x00000000,0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,0x00000000,

    // fill remaining rows with transparent background
};


static uint16_t
gpu_alloc_two_ctrl_ring_desc(void)
{
	static uint8_t	last_idx	= 0;
	uint8_t			current_idx	= last_idx;

	if (!ctrl_queue_bitmap_free) return (uint16_t)-1;

	do
	{
		if (!ctrl_queue_bitmap[current_idx])
		{
			ctrl_queue_bitmap[current_idx] = 1;
			ctrl_queue_bitmap_free--;
			last_idx = current_idx + 1;
			last_idx = last_idx / GPU_QUEUE_BITMAP_SIZE + current_idx % GPU_QUEUE_BITMAP_SIZE;
			return current_idx * 2;
		}
		current_idx++;
		current_idx = current_idx / GPU_QUEUE_BITMAP_SIZE + current_idx % GPU_QUEUE_BITMAP_SIZE;
	} while (current_idx != last_idx);

	return (uint16_t)-1;
}

static void
gpu_free_two_ctrl_ring_desc(uint16_t first_idx)
{
	ctrl_queue_bitmap[first_idx / 2] = 0;
	ctrl_queue_bitmap_free++;
}

uint8_t
gpu_send_ctrl_command(uint32_t cmd, uint32_t cmd_size, uint32_t resp, uint32_t resp_size)
{
	uint16_t	ring_idx					= gpu_alloc_two_ctrl_ring_desc();

	if (ring_idx == (uint16_t)-1) return 0;

	// Note: Set command
	ctrl_queue->desc[ring_idx].addr_low		= cmd;
	ctrl_queue->desc[ring_idx].len			= cmd_size;
	ctrl_queue->desc[ring_idx].flags		= GPU_RING_DESC_NEXT;
	ctrl_queue->desc[ring_idx].next			= ring_idx + 1;

	// Note: Set response
	ctrl_queue->desc[ring_idx + 1].addr_low	= resp;
	ctrl_queue->desc[ring_idx + 1].len		= resp_size;
	ctrl_queue->desc[ring_idx + 1].flags	= GPU_RING_DESC_WRITE;

	// Note: Mark cmd as ready/available
	ctrl_queue->available.ring[ctrl_queue->available.idx % GPU_RING_DESC_CRTLQ_NB] = ring_idx;
	ctrl_queue->available.idx++;

	// Note: notify device
	mmio->queue_notify = 0;

	return 1;
}

void
init_gpu(void)
{
	// Note: Find device
	uint8_t	found = 0;

	for (uint8_t bus = 0; bus < 256; bus++)
	{
		for (uint8_t device = 0; device < 32; device++)
		{
			uint16_t	vendor_id = pci_conf_read16(bus, device, 0, PCI_CONF_VID_OFFSET);

			if (vendor_id == (uint16_t)-1) continue;

			uint16_t	device_id = pci_conf_read16(bus, device, 0, PCI_CONF_DID_OFFSET);

			if (vendor_id == GPU_VENDOR_ID && device_id == GPU_DEVICE_ID)
			{
                printf("Found VirtIO GPU at bus=%d, device=%d\n", bus, device);
				found = 1;
				gpu_bus = bus;
				gpu_device = device;
				break;
			}
		}

		if (found) break;
	}

	if (!found)
	{
		printf("VirtIO GPU not found\n");
		return;
	}

	// Note: Setting up BAR0
	pci_conf_write32(gpu_bus, gpu_device, 0, PCI_CONF_BAR0_OFFSET, 0xFFFFFFFF);
	uint32_t	bar0_size	= pci_conf_read32(gpu_bus, gpu_device, 0, PCI_CONF_BAR0_OFFSET) & ~0xF;
	bar0_size = (~bar0_size) + 1;
	uint32_t	bar0_virt	= kmalloc(bar0_size);
	pci_conf_write32(gpu_bus, gpu_device, 0, PCI_CONF_BAR0_OFFSET, vmm_virt_to_phys(bar0_virt));

	// Note: Enable I/O and memory spaces, bus master
	printf("Enbabling I/O\n");
	uint32_t	bar0 		= pci_conf_read32(gpu_bus, gpu_device, 0, PCI_CONF_BAR0_OFFSET);
	uint16_t	command		= pci_conf_read16(gpu_bus, gpu_device, 0, PCI_CONF_CMDR_OFFSET);
	command |= 7;
	pci_conf_write32(gpu_bus, gpu_device, 0, PCI_CONF_CMDR_OFFSET, command);

	printf("Mapping MMIO\n");
	// Note: Map MMIO (Memory Mapped I/O)
	mmio = (gpu_mmio*)vmm_find_next_frees_kernel(1);
	vmm_map_kpage(bar0, (uint32_t)mmio);

	printf("Reseting device\n");
	// Note: Reset device then acknowledge it
	mmio->status = 0;
	mmio->status = GPU_STATUS_ACK | GPU_STATUS_DRIVER;

	// Note: Set features
	printf("Setting features\n");
	mmio->guest_features = mmio->host_features & GPU_SUPPORTED_FEAT;
	mmio->status |= GPU_STATUS_FEATURES_OK;

	// Note: Set Queue
	printf("Setting queue\n");
	uint32_t	queue_size	= sizeof(gpu_ctrl_queue) / PAGE_SIZE + (sizeof(gpu_ctrl_queue) % PAGE_SIZE ? 1 : 0);
	uint32_t	queue_frame = pmm_alloc_blocks(sizeof(queue_size));
	mmio->queue_sel			= 0;
	mmio->queue_num			= mmio->queue_num_max;
	mmio->queue_align		= PAGE_SIZE;
	mmio->guest_page_size	= PAGE_SIZE;
	mmio->queue_pfn			= queue_frame / PAGE_SIZE;

	ctrl_queue				= (gpu_ctrl_queue*)vmm_find_next_frees_kernel(queue_size);

	for (uint32_t i = 0; i < queue_size * PAGE_SIZE; i+=PAGE_SIZE)
	{
		vmm_map_kpage(queue_frame + i, (uint32_t)ctrl_queue + i);
	}

	memset(ctrl_queue, 0, sizeof(gpu_ctrl_queue));

	mmio->queue_ready		= 1;
	mmio->status 			|= GPU_STATUS_DRIVER_OK;

	// Note: Query display and EDID info
	printf("Querying display\n");
	uint16_t				last_used_idx		= ctrl_queue->used.idx;
	gpu_ctrl_hdr*			cmd_get_edid;
	gpu_resp_edid*			edid;

	gpu_ctrl_hdr*			cmd_display_info	= (gpu_ctrl_hdr*)kmalloc(sizeof(gpu_ctrl_hdr) + sizeof(gpu_resp_display_info));
	gpu_resp_display_info*	display_info 		= (gpu_resp_display_info*)((uint32_t)cmd_display_info + sizeof(gpu_ctrl_hdr));

	if (!cmd_display_info)
	{
		printf("display alloc error");
		return;
	}
	
	memset(cmd_display_info, 0, sizeof(gpu_ctrl_hdr) + sizeof(gpu_resp_display_info));
	printf("memset display\n");

	cmd_display_info->type 						= GPU_CMD_GET_DISPLAY_INFO;
	gpu_send_ctrl_command(
		vmm_virt_to_phys(cmd_display_info), sizeof(gpu_ctrl_hdr),
		vmm_virt_to_phys(display_info), sizeof(gpu_resp_display_info)
	);
	last_used_idx++;
	printf("Display query sent\n");

	if (mmio->guest_features & GPU_FEAT_EDID)
	{
		printf("Querying EDID\n");
		cmd_get_edid		= (gpu_ctrl_hdr*)kmalloc(sizeof(gpu_ctrl_hdr) + sizeof(gpu_resp_edid));
		edid				= (gpu_resp_edid*)((uint32_t)cmd_get_edid + sizeof(gpu_ctrl_hdr));

		memset(cmd_get_edid, 0, sizeof(gpu_ctrl_hdr) + sizeof(gpu_resp_edid));

		cmd_get_edid->type	= GPU_CMD_GET_EDID;
		gpu_send_ctrl_command(
			vmm_virt_to_phys(cmd_get_edid), sizeof(gpu_ctrl_hdr),
			vmm_virt_to_phys(edid), sizeof(gpu_resp_edid)
		);
		last_used_idx++;
		printf("EDID query sent\n");
	}

	printf("Waiting queries to be executed\n");
	// Note: wait for commands completion
	while (ctrl_queue->used.idx < last_used_idx)
	{
		// Failed bit
		if (mmio->status & 0x80)
		{
			printf("queries failed\n");
			return;
		}
	}
	printf("cmd executed\n");

	
	//if ()


	// Note: Init screen

	
	// Note: Init cursor
	// GPU_CMD_RESOURCE_CREATE_2D
	// GPU_CMD_RESOURCE_ATTACH_BACKING
	// GPU_CMD_TRANSFER_TO_HOST_2D
	// GPU_FLAG_FENCE

	// SET
	//GPU_CMD_UPDATE_CURSOR
	// UPDATE
	//GPU_CMD_MOVE_CURSOR
	
}
