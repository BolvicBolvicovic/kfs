#include "vfs.h"

PFILE	root;

uint32_t	fs_read(PFILE node, uint32_t offset, uint32_t size, char* buffer) {
	if (node->node_operations.read) return node->node_operations.read(node, offset, size, buffer);
	return 0;
}

uint32_t	fs_write(PFILE node,uint32_t offset, uint32_t size, char* buffer) {
	if (node->node_operations.write) return node->node_operations.write(node, offset, size, buffer);
	return 0;
}

uint32_t	fs_open(PFILE node, enum open_flags flags) {
	if (node->node_operations.open) return node->node_operations.open(node, flags);
	return 0;
}

uint32_t	fs_close(PFILE node) {
	if (node->node_operations.close) return node->node_operations.close(node);
	return 0;
}

dentry*		fs_readdir(PFILE node, uint32_t index) {
	if ((node->type & FS_DIR) && node->node_operations.readdir) return node->readdir(node, index);
	return NULL;
}
