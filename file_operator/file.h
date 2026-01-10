#ifndef FILE_H
#define FILE_H

#include <stdint.h>

#define FILE_WRITE_OVERWRITE 0
#define FILE_WRITE_APPEND    1

int file_create(uint32_t parent_inode, const char *name);
int file_delete(uint32_t parent_inode, const char *name);

int file_write(uint32_t inode_no,
               const void *buf,
               uint32_t size,
               int mode);

int file_read(uint32_t inode_no,
              void *buf,
              uint32_t size);

#endif
