#ifndef GPU_H
#define GPU_H

#include "../utils.h"
#include "../../memory/vmm/vmm.h"
#include <stdint.h>

#define GPU_VENDOR_ID	0x1AF4
#define GPU_DEVICE_ID	0x1050

#define GPU_FEAT_VIRGL			1
#define GPU_FEAT_EDID			2
#define GPU_FEAT_RESSOURCE_UUID	4
#define GPU_FEAT_RESSOURCE_BLOB	8
#define GPU_FEAT_CONTEXT_INIT	16
#define GPU_SUPPORTED_FEAT		(GPU_FEAT_VIRGL | \
								GPU_FEAT_EDID | \
								GPU_FEAT_RESSOURCE_UUID  | \
								GPU_FEAT_RESSOURCE_BLOB)

#define GPU_STATUS_ACK			1
#define GPU_STATUS_DRIVER		2
#define GPU_STATUS_DRIVER_OK	4
#define GPU_STATUS_FEATURES_OK	8

#define GPU_FALLBACK_WIDTH		1024
#define GPU_FALLBACK_HEIGHT		768
#define GPU_FALLBACK_DISPLAY	0

enum gpu_ctrl_type {
	GPU_UNDEFINED = 0,

	/* 2d commands */
	GPU_CMD_GET_DISPLAY_INFO = 0x0100,
	GPU_CMD_RESOURCE_CREATE_2D,
	GPU_CMD_RESOURCE_UNREF,
	GPU_CMD_SET_SCANOUT,
	GPU_CMD_RESOURCE_FLUSH,
	GPU_CMD_TRANSFER_TO_HOST_2D,
	GPU_CMD_RESOURCE_ATTACH_BACKING,
	GPU_CMD_RESOURCE_DETACH_BACKING,
	GPU_CMD_GET_CAPSET_INFO,
	GPU_CMD_GET_CAPSET,
	GPU_CMD_GET_EDID,
	GPU_CMD_RESOURCE_ASSIGN_UUID,
	GPU_CMD_RESOURCE_CREATE_BLOB,
	GPU_CMD_SET_SCANOUT_BLOB,

	/* 3d commands */
	GPU_CMD_CTX_CREATE = 0x0200,
	GPU_CMD_CTX_DESTROY,
	GPU_CMD_CTX_ATTACH_RESOURCE,
	GPU_CMD_CTX_DETACH_RESOURCE,
	GPU_CMD_RESOURCE_CREATE_3D,
	GPU_CMD_TRANSFER_TO_HOST_3D,
	GPU_CMD_TRANSFER_FROM_HOST_3D,
	GPU_CMD_SUBMIT_3D,
	GPU_CMD_RESOURCE_MAP_BLOB,
	GPU_CMD_RESOURCE_UNMAP_BLOB,

	/* cursor commands */
	GPU_CMD_UPDATE_CURSOR = 0x0300,
	GPU_CMD_MOVE_CURSOR,

	/* success responses */
	GPU_RESP_OK_NODATA = 0x1100,
	GPU_RESP_OK_DISPLAY_INFO,
	GPU_RESP_OK_CAPSET_INFO,
	GPU_RESP_OK_CAPSET,
	GPU_RESP_OK_EDID,
	GPU_RESP_OK_RESOURCE_UUID,
	GPU_RESP_OK_MAP_INFO,

	/* error responses */
	GPU_RESP_ERR_UNSPEC = 0x1200,
	GPU_RESP_ERR_OUT_OF_MEMORY,
	GPU_RESP_ERR_INVALID_SCANOUT_ID,
	GPU_RESP_ERR_INVALID_RESOURCE_ID,
	GPU_RESP_ERR_INVALID_CONTEXT_ID,
	GPU_RESP_ERR_INVALID_PARAMETER,
};

#define GPU_FLAG_FENCE 			1
#define GPU_FLAG_INFO_RING_IDX	2

typedef struct
{
	uint32_t	type;
	uint32_t	flag;
	uint32_t	fence_id;
	uint32_t	ctx_id;
	uint8_t		ring_idx;
	uint8_t		padding[3];
} gpu_ctrl_hdr;

typedef struct 
{
	uint32_t	scanout_id;
	uint32_t	x;
	uint32_t	y;
	uint32_t	padding;
} gpu_cursor_pos;

/* GPU_CMD_UPDATE_CURSOR, GPU_CMD_MOVE_CURSOR */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	gpu_cursor_pos	pos;			/* update & move */
	uint32_t		resource_id;	/* update only */
	uint32_t		hot_x;			/* update only */
	uint32_t		hot_y;			/* update only */
	uint32_t		padding;
} gpu_update_cursor;

/* data passed in the control vq, 2d related */

typedef struct
{
	uint32_t	x;
	uint32_t	y;
	uint32_t	width;
	uint32_t	height;
} gpu_rect;

/* GPU_CMD_RESOURCE_UNREF */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint32_t		resource_id;
	uint32_t		padding;
} gpu_resource_unref;

/* GPU_CMD_RESOURCE_CREATE_2D: create a 2d resource with a format */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint32_t		resource_id;
	uint32_t		format;
	uint32_t		width;
	uint32_t		height;
} gpu_resource_create_2d;

/* GPU_CMD_SET_SCANOUT */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	gpu_rect		r;
	uint32_t		scanout_id;
	uint32_t		resource_id;
} gpu_set_scanout;

/* GPU_CMD_RESOURCE_FLUSH */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	gpu_rect		r;
	uint32_t		resource_id;
	uint32_t		padding;
} gpu_resource_flush;

/* GPU_CMD_TRANSFER_TO_HOST_2D: simple transfer to_host */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	gpu_rect		r;
	uint32_t		offset_high;
	uint32_t		offset_low;
	uint32_t		resource_id;
	uint32_t		padding;
} gpu_transfer_to_host_2d;

typedef struct
{
	uint32_t	addr_high;
	uint32_t	addr_low;
	uint32_t	length;
	uint32_t	padding;
} gpu_mem_entry;

/* GPU_CMD_RESOURCE_ATTACH_BACKING */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint32_t		resource_id;
	uint32_t		nr_entries;
} gpu_resource_attach_backing;

/* GPU_CMD_RESOURCE_DETACH_BACKING */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint32_t		resource_id;
	uint32_t		padding;
} gpu_resource_detach_backing;

/* GPU_RESP_OK_DISPLAY_INFO */
#define GPU_MAX_SCANOUTS 16
typedef struct
{
	gpu_ctrl_hdr	hdr;
	struct gpu_display_one
	{
		gpu_rect	r;
		uint32_t	enabled;
		uint32_t	flags;
	} pmodes[GPU_MAX_SCANOUTS];
} gpu_resp_display_info;

/* data passed in the control vq, 3d related */

typedef struct
{
	uint32_t	x, y, z;
	uint32_t	w, h, d;
} gpu_box;

/* GPU_CMD_TRANSFER_TO_HOST_3D, GPU_CMD_TRANSFER_FROM_HOST_3D */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	gpu_box			box;
	uint32_t 		offset_high;
	uint32_t 		offset_low;
	uint32_t 		resource_id;
	uint32_t 		level;
	uint32_t 		stride;
	uint32_t 		layer_stride;
} gpu_transfer_host_3d;

/* GPU_CMD_RESOURCE_CREATE_3D */
#define GPU_RESOURCE_FLAG_Y_0_TOP (1 << 0)
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint32_t 		resource_id;
	uint32_t 		target;
	uint32_t 		format;
	uint32_t 		bind;
	uint32_t 		width;
	uint32_t 		height;
	uint32_t 		depth;
	uint32_t 		array_size;
	uint32_t 		last_level;
	uint32_t 		nr_samples;
	uint32_t 		flags;
	uint32_t 		padding;
} gpu_resource_create_3d;

/* GPU_CMD_CTX_CREATE */
#define GPU_CONTEXT_INIT_CAPSET_ID_MASK 0x000000ff
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint32_t		nlen;
	uint32_t		context_init;
	char			debug_name[64];
} gpu_ctx_create;

/* GPU_CMD_CTX_DESTROY */
typedef struct
{
	gpu_ctrl_hdr	hdr;
} gpu_ctx_destroy;

/* GPU_CMD_CTX_ATTACH_RESOURCE, GPU_CMD_CTX_DETACH_RESOURCE */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint32_t 		resource_id;
	uint32_t 		padding;
} gpu_ctx_resource;

/* GPU_CMD_SUBMIT_3D */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint32_t 		size;
	uint32_t 		padding;
} gpu_cmd_submit;

#define GPU_CAPSET_VIRGL 1
#define GPU_CAPSET_VIRGL2 2
#define GPU_CAPSET_GFXSTREAM_VULKAN 3
#define GPU_CAPSET_VENUS 4
#define GPU_CAPSET_CROSS_DOMAIN 5
#define GPU_CAPSET_DRM 6

/* GPU_CMD_GET_CAPSET_INFO */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint32_t 		capset_index;
	uint32_t 		padding;
} gpu_get_capset_info;

/* GPU_RESP_OK_CAPSET_INFO */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint32_t		capset_id;
	uint32_t		capset_max_version;
	uint32_t		capset_max_size;
	uint32_t		padding;
} gpu_resp_capset_info;

/* GPU_CMD_GET_CAPSET */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint32_t		capset_id;
	uint32_t		capset_version;
} gpu_get_capset;

/* GPU_RESP_OK_CAPSET */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint8_t*		capset_data;
} gpu_resp_capset;

/* GPU_CMD_GET_EDID */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint32_t		scanout;
	uint32_t		padding;
} gpu_cmd_get_edid;

/* GPU_RESP_OK_EDID */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint32_t		size;
	uint32_t		padding;
	uint8_t			edid[1024];
} gpu_resp_edid;

#define GPU_EVENT_DISPLAY (1 << 0)

typedef struct
{
	uint32_t		events_read;
	uint32_t		events_clear;
	uint32_t		num_scanouts;
	uint32_t		num_capsets;
} gpu_config;

/* simple formats for fbcon/X use */
enum gpu_formats {
	GPU_FORMAT_B8G8R8A8_UNORM  = 1,
	GPU_FORMAT_B8G8R8X8_UNORM  = 2,
	GPU_FORMAT_A8R8G8B8_UNORM  = 3,
	GPU_FORMAT_X8R8G8B8_UNORM  = 4,

	GPU_FORMAT_R8G8B8A8_UNORM  = 67,
	GPU_FORMAT_X8B8G8R8_UNORM  = 68,

	GPU_FORMAT_A8B8G8R8_UNORM  = 121,
	GPU_FORMAT_R8G8B8X8_UNORM  = 134,
};

/* GPU_CMD_RESOURCE_ASSIGN_UUID */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint32_t		resource_id;
	uint32_t		padding;
} gpu_resource_assign_uuid;

/* GPU_RESP_OK_RESOURCE_UUID */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint8_t			uuid[16];
} gpu_resp_resource_uuid;

/* GPU_CMD_RESOURCE_CREATE_BLOB */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint32_t		resource_id;
#define GPU_BLOB_MEM_GUEST             0x0001
#define GPU_BLOB_MEM_HOST3D            0x0002
#define GPU_BLOB_MEM_HOST3D_GUEST      0x0003

#define GPU_BLOB_FLAG_USE_MAPPABLE     0x0001
#define GPU_BLOB_FLAG_USE_SHAREABLE    0x0002
#define GPU_BLOB_FLAG_USE_CROSS_DEVICE 0x0004
	/* zero is invalid blob mem */
	uint32_t 		blob_mem;
	uint32_t 		blob_flags;
	uint32_t 		nr_entries;
	uint32_t		blob_id_high;
	uint32_t		blob_id_low;
	uint32_t		size_high;
	uint32_t		size_low;
	/*
	 * sizeof(nr_entries * gpu_mem_entry) bytes follow
	 */
} gpu_resource_create_blob;

/* GPU_CMD_SET_SCANOUT_BLOB */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	gpu_rect		r;
	uint32_t		scanout_id;
	uint32_t		resource_id;
	uint32_t		width;
	uint32_t		height;
	uint32_t		format;
	uint32_t		padding;
	uint32_t		strides[4];
	uint32_t		offsets[4];
} gpu_set_scanout_blob;

/* GPU_CMD_RESOURCE_MAP_BLOB */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint32_t		resource_id;
	uint32_t		padding;
	uint32_t		offset_high;
	uint32_t		offset_low;
} gpu_resource_map_blob;

/* GPU_RESP_OK_MAP_INFO */
#define GPU_MAP_CACHE_MASK     0x0f
#define GPU_MAP_CACHE_NONE     0x00
#define GPU_MAP_CACHE_CACHED   0x01
#define GPU_MAP_CACHE_UNCACHED 0x02
#define GPU_MAP_CACHE_WC       0x03
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint32_t		map_info;
	uint32_t	 	padding;
} gpu_resp_map_info;

/* GPU_CMD_RESOURCE_UNMAP_BLOB */
typedef struct
{
	gpu_ctrl_hdr	hdr;
	uint32_t 		resource_id;
	uint32_t 		padding;
} gpu_resource_unmap_blob;

typedef struct
{
	uint32_t		addr_high;
	uint32_t		addr_low;
	uint32_t		len;
#define GPU_RING_DESC_NEXT		1
#define GPU_RING_DESC_WRITE		2
#define GPU_RING_DESC_INDIRECT	4
	uint16_t		flags;
	uint16_t		next;
} gpu_ring_desc;

typedef struct
{
	uint32_t		id;
	uint32_t		len;
} gpu_ring_used_elem;

typedef struct
{
#define GPU_RING_DESC_CRTLQ_NB	256
	gpu_ring_desc		desc[GPU_RING_DESC_CRTLQ_NB];
	struct
	{
		uint16_t		flags;
		uint16_t		idx;
		uint16_t		ring[GPU_RING_DESC_CRTLQ_NB];
	} available;
	struct
	{
		uint16_t			flags;
		uint16_t			idx;
		gpu_ring_used_elem	used_elem[GPU_RING_DESC_CRTLQ_NB];
	} used;
} gpu_ctrl_queue;


typedef struct
{
	gpu_ring_desc		desc[16];
	struct
	{
		uint16_t		flags;
		uint16_t		idx;
		uint16_t		ring[16];
	} available;
	struct
	{
		uint16_t			flags;
		uint16_t			idx;
		gpu_ring_used_elem	used_elem[16];
	} used;
} gpu_cursor_queue;

typedef struct
{
    uint32_t		magic_value;
    uint32_t		version;
    uint32_t		device_id;
    uint32_t		vendor_id;
    uint32_t		host_features;
    uint32_t		host_features_sel;
    uint32_t		guest_features;
    uint32_t		guest_features_sel;
    uint32_t		guest_page_size;
    uint32_t		queue_sel;
    uint32_t		queue_num_max;
    uint32_t		queue_num;
    uint32_t		queue_align;
    uint32_t		queue_pfn;
    uint32_t		queue_ready;
    uint32_t		queue_notify;
    uint32_t		interrupt_status;
    uint32_t		interrupt_ack;
    uint32_t		status;
} gpu_mmio;

void	init_gpu(void);

#endif
