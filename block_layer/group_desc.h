#ifndef GROUP_DESC_H
#define GROUP_DESC_H

#include <stdint.h>

/*
 * Group Descriptor (MINI EXT: single group)
 * Mirror metadata and per-group statistics
 */
typedef struct {
    /* Layout (copied from superblock) */
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table_start;

    /* Statistics */
    uint32_t free_blocks_count;
    uint32_t free_inodes_count;
    uint32_t used_dirs_count;
} group_desc_t;

/* API */
int group_desc_init(group_desc_t *gd);
int group_desc_write(const group_desc_t *gd);
int group_desc_read(group_desc_t *gd);
void group_desc_print(const group_desc_t *gd);

#endif
