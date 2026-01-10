#include "super_block.h"
#include "disk.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

int superblock_init(super_block_t *sb,
                    uint32_t total_blocks,
                    uint32_t total_inodes,
                    uint32_t block_size,
                    uint32_t inode_size,
                    const char *volume_name)
{
    if (!sb || !volume_name)
        return -1;

    /* Validate against disk layer */
    if (block_size != BLOCK_SIZE)
        return -1;

    memset(sb, 0, sizeof(super_block_t));

    sb->total_blocks = total_blocks;
    sb->total_inodes = total_inodes;

    sb->free_blocks = total_blocks;
    sb->free_inodes = total_inodes;

    sb->block_size = block_size;
    sb->inode_size = inode_size;

    /* MINI EXT: single block group */
    sb->blocks_per_group = total_blocks;
    sb->inodes_per_group = total_inodes;

    /*
     * Fixed layout (single group):
     * block 0 : unused
     * block 1 : superblock
     * block 2 : group descriptor
     * block 3 : inode bitmap
     * block 4 : block bitmap
     * block 5 : inode table start
     */
    sb->inode_bitmap = 3;
    sb->block_bitmap = 4;
    sb->inode_table_start  = 5;

    /* Compute first data block */
    uint32_t inode_table_blocks =
        (total_inodes * inode_size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    sb->first_data_block = sb->inode_table_start + inode_table_blocks;

    /* Update free blocks (exclude metadata) */
    sb->free_blocks -= sb->first_data_block;

    /* UUID */
    srand((unsigned int)time(NULL));
    for (int i = 0; i < 16; i++)
        sb->fs_uuid[i] = rand() & 0xFF;

    strncpy(sb->volume_name, volume_name, VOLUME_NAME_LEN - 1);
    sb->volume_name[VOLUME_NAME_LEN - 1] = '\0';

    sb->magic_number = FS_MAGIC;

    return 0;
}

int superblock_write(const super_block_t *sb)
{
    uint8_t buffer[BLOCK_SIZE];

    if (!sb)
        return -1;

    memset(buffer, 0, BLOCK_SIZE);
    memcpy(buffer, sb, sizeof(super_block_t));

    return (disk_write(SUPERBLOCK_BLOCK, buffer) < 0) ? -1 : 0;
}

int superblock_read(super_block_t *sb)
{
    uint8_t buffer[BLOCK_SIZE];

    if (!sb)
        return -1;

    if (disk_read(SUPERBLOCK_BLOCK, buffer) < 0)
        return -1;

    memcpy(sb, buffer, sizeof(super_block_t));

    if (sb->magic_number != FS_MAGIC)
        return -1;

    return 0;
}

void superblock_print(const super_block_t *sb)
{
    printf("=== SUPER BLOCK ===\n");
    printf("Volume name        : %s\n", sb->volume_name);
    printf("Magic              : 0x%x\n", sb->magic_number);
    printf("Block size         : %u\n", sb->block_size);
    printf("Inode size         : %u\n", sb->inode_size);
    printf("Total blocks       : %u\n", sb->total_blocks);
    printf("Free blocks        : %u\n", sb->free_blocks);
    printf("Total inodes       : %u\n", sb->total_inodes);
    printf("Free inodes        : %u\n", sb->free_inodes);
    printf("Inode bitmap block : %u\n", sb->inode_bitmap);
    printf("Block bitmap block : %u\n", sb->block_bitmap);
    printf("Inode table start  : %u\n", sb->inode_table_start);
    printf("First data block   : %u\n", sb->first_data_block);
}
