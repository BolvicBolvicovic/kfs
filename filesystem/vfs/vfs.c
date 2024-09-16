#include "vfs.h"

fs_node_t   *fs_root = NULL;

uint32_t        fs_write(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {

}

uint32_t        fs_read(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {

}

void            fs_open(fs_node_t* node, uint8_t read, uint8_t write) {

}

void            fs_close(fs_node_t* node) {

}

dir_entry_t*    readdir_fs(fs_node_t* node, uint32_t index) {

}

fs_node_t*      finddir_fs(fs_node_t*, char* name) {

}
