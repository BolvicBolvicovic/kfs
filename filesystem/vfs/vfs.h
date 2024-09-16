#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>

enum fs_flags {
    FS_FILE = 1,
    FS_DIRECTORY = 2,
    FS_CHARDEVICE = 3,
    FS_BLOCKDEVICE = 4,
    FS_PIPE = 5,
    FS_SYMLINK = 6,
    FS_MOUNTPOINT = 8
};

typedef struct fs_node_s {
    char        name[128]; // Should move it to directory node
    uint32_t    permissions;
    uint32_t    uid;
    uint32_t    gid;
    uint32_t    flags;
    uint32_t    inode; // Provide to a filesystem a way to identify nodes
    uint32_t    len;
    uint32_t    impl; // Implementation-defined number
    // These are pointers to callbacks
    rw_t        read;
    rw_t        write;
    oc_t        open;
    oc_t        close;
    readdir_t   readdir;
    finddir_t   finddir;
    struct fs_node_s *ptr; // For symlink and mountpoint
} fs_node_t;

typedef struct {
    char        name[128];
    uint32_t    inode;
} dir_entry_t;

typedef uint32_t      (*rw_t)(fs_node_t*, uint32_t, uint32_t, uint8_t*);
typedef void          (*oc_t)(fs_node_t*);
typedef dir_entry_t * (*readdir_t)(fs_node_t*, uint32_t);  
typedef fs_node_t *   (*finddir_t)(fs_node_t*, char* name);

uint32_t        fs_write(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
uint32_t        fs_read(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
void            fs_open(fs_node_t* node, uint8_t read, uint8_t write);
void            fs_close(fs_node_t* node);
dir_entry_t*    readdir_fs(fs_node_t* node, uint32_t index);
fs_node_t*      finddir_fs(fs_node_t*, char* name);

#endif
