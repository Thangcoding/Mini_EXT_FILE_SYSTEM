#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "file.h"
#include "../directory/directory.h"
#include "../directory/dir_entry.h"
#include "../block_layer/inode_table.h"
#include "../block_layer/inode.h"
#include "../block_layer/bitmap.h"
#include "../block_layer/group_desc.h"
#include "../block_layer/disk.h"
#include "../block_layer/super_block.h"


static int file_add_dir_entry(uint32_t dir_inode,
                              const char *name,
                              uint32_t inode_no)
{
    inode_t dir;
    dir_block_t *blk;

    if (dir_load(dir_inode, &dir, &blk) < 0)
        return -1;

    for (uint32_t i = 0; i < blk->entries_count; i++) {
        if (strcmp(blk->entries[i].name, name) == 0) {
            free(blk);
            return -1;
        }
    }

    blk = realloc(blk, sizeof(dir_block_t) +
                        (blk->entries_count + 1) * sizeof(dir_entry_t));

    dir_entry_t *e = &blk->entries[blk->entries_count];
    memset(e, 0, sizeof(*e));

    strncpy(e->name, name, DIR_NAME_LEN);
    e->name_len = strlen(e->name);
    e->inode = inode_no;
    e->file_type = FILE_TYPE_REGULAR;

    blk->entries_count++;
    dir.file_size += sizeof(dir_entry_t);

    dir_sync(&dir, blk);
    inode_write(dir_inode, &dir);

    free(blk);
    return 0;
}

static int file_remove_dir_entry(uint32_t dir_inode, const char *name)
{
    inode_t dir;
    dir_block_t *blk;

    if (dir_load(dir_inode, &dir, &blk) < 0)
        return -1;

    int idx = -1;
    for (uint32_t i = 0; i < blk->entries_count; i++) {
        if (strcmp(blk->entries[i].name, name) == 0) {
            idx = i;
            break;
        }
    }

    if (idx < 0) {
        free(blk);
        return -1;
    }

    for (uint32_t i = idx; i < blk->entries_count - 1; i++)
        blk->entries[i] = blk->entries[i + 1];

    blk->entries_count--;
    dir.file_size -= sizeof(dir_entry_t);

    dir_sync(&dir, blk);
    inode_write(dir_inode, &dir);

    free(blk);
    return 0;
}


int file_create(uint32_t parent_inode, const char *name)
{
    uint32_t ino;

    if (inode_table_alloc(&ino) < 0)
        return -1;

    inode_t inode;
    inode_init(&inode, ino, INODE_FILE, 0644);
    inode_write(ino, &inode);

    if (file_add_dir_entry(parent_inode, name, ino) < 0) {
        inode_table_free(ino);
        return -1;
    }

    return 0;
}


int file_delete(uint32_t parent_inode, const char *name)
{
    dir_entry_t e;
    if (directory_lookup(parent_inode, name, &e) < 0)
        return -1;

    inode_t inode;
    inode_read(e.inode, &inode);

    if (inode.file_type != INODE_FILE)
        return -1;

    super_block_t sb;
    group_desc_t gd;
    superblock_read(&sb);
    group_desc_read(&gd);

    for (int i = 0; i < INODE_DIRECT_PTRS; i++) {
        if (inode.blocks[i]) {
            bitmap_clear(gd.block_bitmap,
                         inode.blocks[i],
                         sb.total_blocks);
            gd.free_blocks_count++;
        }
    }

    inode_table_free(e.inode);
    file_remove_dir_entry(parent_inode, name);

    group_desc_write(&gd);
    return 0;
}

int file_write(uint32_t inode_no,
               const void *buf,
               uint32_t size,
               int mode)
{
    inode_t inode;
    inode_read(inode_no, &inode);

    if (inode.file_type != INODE_FILE)
        return -1;

    super_block_t sb;
    group_desc_t gd;
    superblock_read(&sb);
    group_desc_read(&gd);

    uint32_t offset = 0;
    if (mode == FILE_WRITE_APPEND)
        offset = inode.file_size;
    else {
        for (int i = 0; i < INODE_DIRECT_PTRS; i++) {
            if (inode.blocks[i]) {
                bitmap_clear(gd.block_bitmap,
                             inode.blocks[i],
                             sb.total_blocks);
                gd.free_blocks_count++;
                inode.blocks[i] = 0;
            }
        }
        inode.file_size = 0;
    }

    const uint8_t *p = buf;
    uint32_t remaining = size;

    while (remaining > 0) {
        uint32_t bi = offset / BLOCK_SIZE;
        uint32_t bo = offset % BLOCK_SIZE;

        if (bi >= INODE_DIRECT_PTRS)
            return -1;

        if (inode.blocks[bi] == 0) {
            int b = bitmap_alloc(gd.block_bitmap,
                                 sb.first_data_block,
                                 sb.total_blocks);
            if (b < 0)
                return -1;

            inode.blocks[bi] = b;
            gd.free_blocks_count--;
        }

        uint8_t blk[BLOCK_SIZE];
        disk_read(inode.blocks[bi], blk);

        uint32_t copy = BLOCK_SIZE - bo;
        if (copy > remaining)
            copy = remaining;

        memcpy(blk + bo, p, copy);
        disk_write(inode.blocks[bi], blk);

        offset += copy;
        p += copy;
        remaining -= copy;
    }

    inode.file_size = offset;
    inode_write(inode_no, &inode);
    group_desc_write(&gd);

    return 0;
}

int file_read(uint32_t inode_no,
              void *buf,
              uint32_t size)
{
    inode_t inode;
    inode_read(inode_no, &inode);

    if (inode.file_type != INODE_FILE)
        return -1;

    uint8_t *p = buf;
    uint32_t offset = 0;
    uint32_t remain = size;

    while (remain > 0 && offset < inode.file_size) {
        uint32_t bi = offset / BLOCK_SIZE;
        uint32_t bo = offset % BLOCK_SIZE;

        if (bi >= INODE_DIRECT_PTRS || inode.blocks[bi] == 0)
            break;

        uint8_t blk[BLOCK_SIZE];
        disk_read(inode.blocks[bi], blk);

        uint32_t copy = BLOCK_SIZE - bo;
        if (copy > remain)
            copy = remain;
        if (copy > inode.file_size - offset)
            copy = inode.file_size - offset;

        memcpy(p, blk + bo, copy);

        p += copy;
        offset += copy;
        remain -= copy;
    }

    return offset;
}
