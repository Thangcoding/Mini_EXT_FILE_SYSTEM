#ifndef INODE_TABLE_H
#define INODE_TABLE_H

#include <stdint.h>
#include "inode.h"

int inode_table_alloc(uint32_t *inode_number);
int inode_table_free(uint32_t inode_number);

int inode_table_read(uint32_t inode_number, inode_t *inode);
int inode_table_write(uint32_t inode_number, const inode_t *inode);

#endif
