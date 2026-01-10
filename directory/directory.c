#include "directory.h"

#include "../block_layer/disk.h"
#include "../block_layer/bitmap.h"
#include "../block_layer/inode.h"
#include "../block_layer/inode_table.h"
#include "../block_layer/super_block.h"
#include "../block_layer/group_desc.h"
#include "../block_layer/inode.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int dir_load(uint32_t inode_no, inode_t *inode, dir_block_t **dir)
{
    if (inode_read(inode_no, inode) < 0)
        return -1;

    if (inode->file_type != INODE_DIR)
        return -1;

    if (inode->blocks[0] == 0)
        return -1;

    uint8_t buf[BLOCK_SIZE];
    if (disk_read(inode->blocks[0], buf) < 0)
        return -1;

    uint32_t count;
    memcpy(&count, buf, sizeof(uint32_t));

    if (count > (BLOCK_SIZE - sizeof(uint32_t)) / sizeof(dir_entry_t))
        return -1;

    *dir = malloc(sizeof(dir_block_t) + count * sizeof(dir_entry_t));
    memcpy(*dir, buf, sizeof(uint32_t) + count * sizeof(dir_entry_t));
    return 0;
}

int dir_sync(const inode_t *inode, const dir_block_t *dir)
{
    uint32_t size = sizeof(uint32_t) +
                    dir->entries_count * sizeof(dir_entry_t);

    if (size > BLOCK_SIZE)
        return -1;

    uint8_t buf[BLOCK_SIZE];
    memset(buf, 0, BLOCK_SIZE);
    memcpy(buf, dir, size);

    return disk_write(inode->blocks[0], buf);
}


int directory_create(uint32_t parent_inode, const char *name)
{
    if (!name || strlen(name) == 0 || strlen(name) > DIR_NAME_LEN)
        return -1;

    inode_t parent;
    dir_block_t *parent_dir;

    if (dir_load(parent_inode, &parent, &parent_dir) < 0)
        return -1;

    /* Check duplicate */
    for (uint32_t i = 0; i < parent_dir->entries_count; i++) {
        if (strcmp(parent_dir->entries[i].name, name) == 0) {
            free(parent_dir);
            return -1;
        }
    }

    super_block_t sb;
    group_desc_t gd;
    superblock_read(&sb);
    group_desc_read(&gd);

    /* Allocate inode */
    uint32_t new_ino;
    if (inode_table_alloc(&new_ino) < 0) {
        free(parent_dir);
        return -1;
    }

    /* Allocate block */
    int block = bitmap_alloc(gd.block_bitmap, sb.first_data_block,sb.total_blocks);
    if (block < 0) {
        inode_table_free(new_ino);
        free(parent_dir);
        return -1;
    }

    /* Init inode */
    inode_t new_inode;
    inode_init(&new_inode, new_ino, INODE_DIR, 0755);
    new_inode.blocks[0] = block;
    new_inode.file_size =
        sizeof(uint32_t) + 2 * sizeof(dir_entry_t);
    inode_write(new_ino, &new_inode);

    /* Init "." and ".." */
    dir_block_t *new_dir =
        malloc(sizeof(dir_block_t) + 2 * sizeof(dir_entry_t));
    new_dir->entries_count = 2;

    strcpy(new_dir->entries[0].name, ".");
    new_dir->entries[0].inode = new_ino;
    new_dir->entries[0].file_type = FILE_TYPE_DIRECTORY;
    new_dir->entries[0].name_len = 1;

    strcpy(new_dir->entries[1].name, "..");
    new_dir->entries[1].inode = parent_inode;
    new_dir->entries[1].file_type = FILE_TYPE_DIRECTORY;
    new_dir->entries[1].name_len = 2;

    dir_sync(&new_inode, new_dir);
    free(new_dir);

    /* Add entry to parent */
    parent_dir = realloc(parent_dir,
        sizeof(dir_block_t) +
        (parent_dir->entries_count + 1) * sizeof(dir_entry_t));

    dir_entry_t *e =
        &parent_dir->entries[parent_dir->entries_count];
    memset(e, 0, sizeof(*e));

    strncpy(e->name, name, DIR_NAME_LEN);
    e->inode = new_ino;
    e->file_type = FILE_TYPE_DIRECTORY;
    e->name_len = strlen(e->name);

    parent_dir->entries_count++;
    parent.file_size += sizeof(dir_entry_t);

    dir_sync(&parent, parent_dir);
    inode_write(parent_inode, &parent);

    free(parent_dir);

    gd.free_blocks_count--;
    gd.used_dirs_count++;
    group_desc_write(&gd);

    return 0;
}

int directory_delete(uint32_t parent_inode, const char *name)
{
    if (!strcmp(name, ".") || !strcmp(name, ".."))
        return -1;

    inode_t parent;
    dir_block_t *parent_dir;

    if (dir_load(parent_inode, &parent, &parent_dir) < 0)
        return -1;

    int idx = -1;
    dir_entry_t target;

    for (uint32_t i = 0; i < parent_dir->entries_count; i++) {
        if (strcmp(parent_dir->entries[i].name, name) == 0) {
            idx = i;
            target = parent_dir->entries[i];
            break;
        }
    }

    if (idx < 0) {
        free(parent_dir);
        return -1;
    }

    inode_t victim;
    dir_block_t *victim_dir;

    if (dir_load(target.inode, &victim, &victim_dir) < 0) {
        free(parent_dir);
        return -1;
    }

    if (victim_dir->entries_count > 2) {
        free(victim_dir);
        free(parent_dir);
        return -1;
    }

    free(victim_dir);
    super_block_t sb;
    group_desc_t gd;

    superblock_read(&sb);
    group_desc_read(&gd);

    bitmap_clear(gd.block_bitmap, victim.blocks[0], sb.total_blocks);
    inode_table_free(target.inode);

    gd.free_blocks_count++;
    gd.used_dirs_count--;
    group_desc_write(&gd);

    for (uint32_t i = idx; i < parent_dir->entries_count - 1; i++)
        parent_dir->entries[i] = parent_dir->entries[i + 1];

    parent_dir->entries_count--;
    parent.file_size -= sizeof(dir_entry_t);

    dir_sync(&parent, parent_dir);
    inode_write(parent_inode, &parent);

    free(parent_dir);
    return 0;
}

int directory_lookup(uint32_t dir_inode,
                     const char *name,
                     dir_entry_t *result)
{
    inode_t inode;
    dir_block_t *dir;

    if (dir_load(dir_inode, &inode, &dir) < 0)
        return -1;

    for (uint32_t i = 0; i < dir->entries_count; i++) {
        if (strcmp(dir->entries[i].name, name) == 0) {
            *result = dir->entries[i];
            free(dir);
            return 0;
        }
    }

    free(dir);
    return -1;
}

int directory_list(uint32_t dir_inode)
{
    inode_t inode;
    dir_block_t *dir;

    if (dir_load(dir_inode, &inode, &dir) < 0)
        return -1;

    for (uint32_t i = 0; i < dir->entries_count; i++)
        printf("%s  ", dir->entries[i].name);

    printf("\n");
    free(dir);
    return 0;
}
