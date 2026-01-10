#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <stdint.h>

#define FS_MAGIC 0xEF53
#define VOLUME_NAME_LEN 32

#define SUPERBLOCK_BLOCK 1   // block index of superblock

typedef struct {
    /* Basic FS info */
    uint32_t total_blocks;
    uint32_t total_inodes;

    uint32_t free_blocks;
    uint32_t free_inodes;

    uint32_t block_size;
    uint32_t inode_size;

    /* Single block group (MINI EXT) */
    uint32_t blocks_per_group;
    uint32_t inodes_per_group;

    /* Layout information */
    uint32_t first_data_block;

    uint32_t inode_bitmap;
    uint32_t block_bitmap;
    uint32_t inode_table_start;

    /* Identity */
    uint8_t  fs_uuid[16];
    char     volume_name[VOLUME_NAME_LEN];

    uint32_t magic_number;
} super_block_t;

/* API */
int superblock_init(super_block_t *sb,
                    uint32_t total_blocks,
                    uint32_t total_inodes,
                    uint32_t block_size,
                    uint32_t inode_size,
                    const char *volume_name);

int superblock_write(const super_block_t *sb);
int superblock_read(super_block_t *sb);
void superblock_print(const super_block_t *sb);

#endif
