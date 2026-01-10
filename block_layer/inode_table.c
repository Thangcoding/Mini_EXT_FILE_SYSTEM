#include "inode_table.h"
#include "bitmap.h"
#include "super_block.h"
#include "group_desc.h"
#include "inode.h"

#include <string.h>

/*
 * Allocate a new inode number
 */
int inode_table_alloc(uint32_t *inode_number)
{
    super_block_t sb;
    group_desc_t gd;

    if (!inode_number)
        return -1;

    if (superblock_read(&sb) < 0)
        return -1;

    if (group_desc_read(&gd) < 0)
        return -1;

    if (sb.free_inodes == 0)
        return -1;

    /* Allocate inode number from inode bitmap */
    int ino = bitmap_alloc(gd.inode_bitmap,
                           0,
                           sb.total_inodes);
    if (ino < 0)
        return -1;

    /* Zero inode on disk (do NOT set type here) */
    inode_t inode;
    memset(&inode, 0, sizeof(inode_t));
    inode.inode_number = ino;

    if (inode_write(ino, &inode) < 0) {
        /* rollback bitmap */
        bitmap_clear(gd.inode_bitmap, ino, sb.total_inodes);
        return -1;
    }

    /* Update counters */
    sb.free_inodes--;
    gd.free_inodes_count--;

    superblock_write(&sb);
    group_desc_write(&gd);

    *inode_number = ino;
    return 0;
}

/*
 * Free an inode
 */
int inode_table_free(uint32_t inode_number)
{
    super_block_t sb;
    group_desc_t gd;

    if (superblock_read(&sb) < 0)
        return -1;

    if (group_desc_read(&gd) < 0)
        return -1;

    if (inode_number >= sb.total_inodes)
        return -1;

    int used = bitmap_test(gd.inode_bitmap,
                           inode_number,
                           sb.total_inodes);
    if (used != 1)
        return -1;

    /* Clear inode bitmap */
    if (bitmap_clear(gd.inode_bitmap,
                     inode_number,
                     sb.total_inodes) < 0)
        return -1;

    /* Zero inode on disk */
    inode_t inode;
    memset(&inode, 0, sizeof(inode_t));
    inode_write(inode_number, &inode);

    sb.free_inodes++;
    gd.free_inodes_count++;

    superblock_write(&sb);
    group_desc_write(&gd);

    return 0;
}

/*
 * Read inode
 */
int inode_table_read(uint32_t inode_number, inode_t *inode)
{
    return inode_read(inode_number, inode);
}

/*
 * Write inode
 */
int inode_table_write(uint32_t inode_number, const inode_t *inode)
{
    return inode_write(inode_number, inode);
}
