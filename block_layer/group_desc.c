#include "group_desc.h"
#include "super_block.h"
#include "disk.h"

#include <string.h>
#include <stdio.h>

#define GROUP_DESC_BLOCK 2   /* fixed for MINI EXT layout */

int group_desc_init(group_desc_t *gd)
{
    super_block_t sb;

    if (!gd)
        return -1;

    if (superblock_read(&sb) < 0)
        return -1;

    memset(gd, 0, sizeof(group_desc_t));

    /* Copy layout info from superblock */
    gd->block_bitmap = sb.block_bitmap;
    gd->inode_bitmap = sb.inode_bitmap;
    gd->inode_table_start  = sb.inode_table_start;

    /* Mirror counters */
    gd->free_blocks_count = sb.free_blocks;
    gd->free_inodes_count = sb.free_inodes;

    /* Root directory exists */
    gd->used_dirs_count = 1;

    return 0;
}

int group_desc_write(const group_desc_t *gd)
{
    uint8_t buffer[BLOCK_SIZE];

    if (!gd)
        return -1;

    memset(buffer, 0, BLOCK_SIZE);
    memcpy(buffer, gd, sizeof(group_desc_t));

    return (disk_write(GROUP_DESC_BLOCK, buffer) < 0) ? -1 : 0;
}

int group_desc_read(group_desc_t *gd)
{
    uint8_t buffer[BLOCK_SIZE];

    if (!gd)
        return -1;

    if (disk_read(GROUP_DESC_BLOCK, buffer) < 0)
        return -1;

    memcpy(gd, buffer, sizeof(group_desc_t));
    return 0;
}

void group_desc_print(const group_desc_t *gd)
{
    printf("=== GROUP DESCRIPTOR ===\n");
    printf("Block bitmap block : %u\n", gd->block_bitmap);
    printf("Inode bitmap block : %u\n", gd->inode_bitmap);
    printf("Inode table start  : %u\n", gd->inode_table_start);
    printf("Free blocks        : %u\n", gd->free_blocks_count);
    printf("Free inodes        : %u\n", gd->free_inodes_count);
    printf("Used directories   : %u\n", gd->used_dirs_count);
}
