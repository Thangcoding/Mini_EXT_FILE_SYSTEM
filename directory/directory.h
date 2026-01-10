#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <stdint.h>
#include "dir_entry.h"
#include "../block_layer/inode.h"

typedef struct {
    uint32_t entries_count;
    dir_entry_t entries[];
} dir_block_t;

/* Directory operations */
int dir_sync(const inode_t *inode, const dir_block_t *dir);
int dir_load(uint32_t inode_no, inode_t *inode, dir_block_t **dir);
int directory_create(uint32_t parent_inode, const char *name);
int directory_delete(uint32_t parent_inode, const char *name);

int directory_lookup(uint32_t dir_inode,
                     const char *name,
                     dir_entry_t *result);

int directory_list(uint32_t dir_inode);

#endif
