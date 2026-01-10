#include "inode.h"
#include "disk.h"
#include "super_block.h"

#include <string.h>
#include <stdio.h>

void inode_init(inode_t *inode,
                uint32_t inode_number,
                uint16_t file_type,
                uint16_t permissions)
{
    memset(inode, 0, sizeof(inode_t));

    inode->inode_number = inode_number;
    inode->file_type    = file_type;
    inode->permissions  = permissions;
    inode->file_size    = 0;

    inode->single_indirect = 0;
    inode->double_indirect = 0;
}

/* Read inode from inode table */
int inode_read(uint32_t inode_number, inode_t *inode)
{
    super_block_t sb;
    uint8_t block_buf[BLOCK_SIZE];

    if (!inode)
        return -1;

    if (superblock_read(&sb) < 0)
        return -1;

    if (inode_number >= sb.total_inodes)
        return -1;

    uint32_t inode_size = sb.inode_size;
    uint32_t inodes_per_block = BLOCK_SIZE / inode_size;

    uint32_t block =
        sb.inode_table_start + (inode_number / inodes_per_block);

    uint32_t offset =
        (inode_number % inodes_per_block) * inode_size;

    if (disk_read(block, block_buf) < 0)
        return -1;

    /* Copy exactly inode_size bytes */
    memcpy(inode, block_buf + offset, inode_size);

    return 0;
}

/* Write inode to inode table */
int inode_write(uint32_t inode_number, const inode_t *inode)
{
    super_block_t sb;
    uint8_t block_buf[BLOCK_SIZE];

    if (!inode)
        return -1;

    if (superblock_read(&sb) < 0)
        return -1;

    if (inode_number >= sb.total_inodes)
        return -1;

    uint32_t inode_size = sb.inode_size;
    uint32_t inodes_per_block = BLOCK_SIZE / inode_size;

    uint32_t block =
        sb.inode_table_start + (inode_number / inodes_per_block);

    uint32_t offset =
        (inode_number % inodes_per_block) * inode_size;

    if (disk_read(block, block_buf) < 0)
        return -1;

    memcpy(block_buf + offset, inode, inode_size);

    return (disk_write(block, block_buf) < 0) ? -1 : 0;
}

void inode_print(const inode_t *inode)
{
    printf("=== INODE %u ===\n", inode->inode_number);
    printf("Type        : %s\n",
           inode->file_type == INODE_DIR ? "DIR" : "FILE");
    printf("Permissions : %o\n", inode->permissions);
    printf("File size   : %u\n", inode->file_size);

    for (int i = 0; i < INODE_DIRECT_PTRS; i++)
        printf("Block[%d]    : %u\n", i, inode->blocks[i]);

    printf("Single ind  : %u\n", inode->single_indirect);
    printf("Double ind  : %u\n", inode->double_indirect);
}

