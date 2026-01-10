#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "block_layer/disk.h"
#include "block_layer/super_block.h"
#include "block_layer/group_desc.h"
#include "block_layer/bitmap.h"
#include "block_layer/inode.h"
#include "block_layer/inode_table.h"
#include "directory/dir_entry.h"

/* ===== FS CONFIG ===== */
#define TOTAL_BLOCKS 1024
#define TOTAL_INODES 128
#define DISK_IMAGE   "disk.img"

/* inode 0 RESERVED, inode 1 ROOT */
#define ROOT_INODE   1

int main()
{
    printf("[mkfs] Formatting filesystem...\n");

    /* ===== CREATE & SIZE DISK IMAGE ===== */
    int fd = open(DISK_IMAGE, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    if (ftruncate(fd, TOTAL_BLOCKS * BLOCK_SIZE) < 0) {
        perror("ftruncate");
        close(fd);
        return 1;
    }
    close(fd);

    if (disk_open(DISK_IMAGE) < 0) {
        printf("mkfs: cannot open disk\n");
        return 1;
    }

    /* ===== SUPERBLOCK ===== */
    super_block_t sb;
    if (superblock_init(&sb,
                         TOTAL_BLOCKS,
                         TOTAL_INODES,
                         BLOCK_SIZE,
                         sizeof(inode_t),
                         "mini-ext") < 0) {
        printf("mkfs: superblock_init failed\n");
        return 1;
    }
    superblock_write(&sb);

    /* ===== GROUP DESCRIPTOR ===== */
    group_desc_t gd;
    memset(&gd, 0, sizeof(gd));

    gd.inode_bitmap       = sb.inode_bitmap;
    gd.block_bitmap       = sb.block_bitmap;
    gd.inode_table_start  = sb.inode_table_start;
    gd.free_blocks_count  = sb.free_blocks;
    gd.free_inodes_count  = sb.free_inodes;
    gd.used_dirs_count    = 1;   /* root */

    group_desc_write(&gd);

    /* ===== CLEAR BITMAPS ===== */
    uint8_t zero[BLOCK_SIZE];
    memset(zero, 0, BLOCK_SIZE);

    disk_write(sb.inode_bitmap, zero);
    disk_write(sb.block_bitmap, zero);

    /* ===== MARK RESERVED BLOCKS ===== */
    for (uint32_t i = 0; i < sb.first_data_block; i++)
        bitmap_set(sb.block_bitmap, i, sb.total_blocks);

    /* ===== MARK ROOT INODE (inode 1) ===== */
    bitmap_set(sb.inode_bitmap, ROOT_INODE, sb.total_inodes);
    sb.free_inodes--;
    gd.free_inodes_count--;

    /* ===== ALLOC ROOT DATA BLOCK ===== */
    int root_block = bitmap_alloc(sb.block_bitmap,
                                  sb.first_data_block,
                                  sb.total_blocks);
    if (root_block < 0) {
        printf("mkfs: cannot allocate root block\n");
        return 1;
    }

    sb.free_blocks--;
    gd.free_blocks_count--;

    superblock_write(&sb);
    group_desc_write(&gd);

    /* ===== INIT ROOT INODE ===== */
    inode_t root;
    inode_init(&root, ROOT_INODE, INODE_DIR, 0755);
    root.blocks[0] = root_block;
    root.file_size = sizeof(uint32_t) + 2 * sizeof(dir_entry_t);

    inode_write(ROOT_INODE, &root);

    /* ===== INIT ROOT DIRECTORY CONTENT ===== */
    uint8_t buf[BLOCK_SIZE];
    memset(buf, 0, BLOCK_SIZE);

    uint32_t count = 2;
    memcpy(buf, &count, sizeof(uint32_t));

    dir_entry_t *e = (dir_entry_t *)(buf + sizeof(uint32_t));

    strcpy(e[0].name, ".");
    e[0].inode = ROOT_INODE;
    e[0].file_type = FILE_TYPE_DIRECTORY;
    e[0].name_len = 1;

    strcpy(e[1].name, "..");
    e[1].inode = ROOT_INODE;
    e[1].file_type = FILE_TYPE_DIRECTORY;
    e[1].name_len = 2;

    disk_write(root_block, buf);

    printf("[mkfs] Done. Root inode = %d, root block = %d\n",
           ROOT_INODE, root_block);

    disk_close();
    return 0;
}
