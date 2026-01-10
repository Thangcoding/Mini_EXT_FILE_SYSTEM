#ifndef INODE_H
#define INODE_H

#include <stdint.h>

#define INODE_DIRECT_PTRS 12

#define INODE_FILE 1
#define INODE_DIR  2

typedef struct {
    uint32_t inode_number;
    uint32_t file_size;

    uint32_t blocks[INODE_DIRECT_PTRS];
    uint32_t single_indirect;
    uint32_t double_indirect;

    uint16_t file_type;
    uint16_t permissions;
} inode_t;

/* API */
void inode_init(inode_t *inode,
                uint32_t inode_number,
                uint16_t file_type,
                uint16_t permissions);

int inode_read(uint32_t inode_number, inode_t *inode);
int inode_write(uint32_t inode_number, const inode_t *inode);

void inode_print(const inode_t *inode);

#endif
