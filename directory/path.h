#ifndef PATH_H
#define PATH_H

#include <stdint.h>

/*
 * Resolve a path to an inode number
 * path        : absolute or relative path
 * cwd_inode  : current working directory inode
 * result_inode: resolved inode
 */
int path_resolve(const char *path,
                 uint32_t cwd_inode,
                 uint32_t *result_inode);

/*
 * Resolve parent directory of a path
 * path         : input path
 * cwd_inode   : current working directory inode
 * parent_inode: output parent inode
 * name        : output last component (DIR_NAME_LEN + 1)
 */
int path_parent(const char *path,
                uint32_t cwd_inode,
                uint32_t *parent_inode,
                char *name);

#endif
