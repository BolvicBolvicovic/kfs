#include "vfs.h"

struct fs_node_s   *fs_root = NULL;

uint32_t        fs_write(struct fs_node_s* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
	if (!node->write) return node->write(node, offset, size, buffer);
	return 0;
}

uint32_t        fs_read(struct fs_node_s* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
	if (!node->read) return node->read(node, offset, size, buffer);
	return 0;
}

void            fs_open(struct fs_node_s* node, uint8_t read, uint8_t write) {
	if (!node->open) (node->open(node));
}

void            fs_close(struct fs_node_s* node) {
	if (!node->close) (node->close(node));
}

dir_entry_t*    readdir_fs(struct fs_node_s* node, uint32_t index) {
	if (!node->readdir && (node->flags & 7 == FS_DIRECTORY)) return node->readdir(node, index);
	return NULL;
}

struct fs_node_s*      finddir_fs(struct fs_node_s* node, char* name) {
	if (!node->finddir && (node->flags & 7 == FS_DIRECTORY)) return node->finddir(node, name);
	return NULL;
}
