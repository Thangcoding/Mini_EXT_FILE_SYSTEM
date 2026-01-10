#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "path.h"
#include "directory.h"
#include "dir_entry.h"

/*
 * IMPORTANT INVARIANT
 * inode 0 is RESERVED
 * inode 1 is ROOT
 */
#define ROOT_INODE   1
#define MAX_PATH_LEN 1024

int path_resolve(const char *path,
                 uint32_t cwd_inode,
                 uint32_t *result_inode)
{
    char buf[MAX_PATH_LEN];
    char *token, *saveptr;
    uint32_t current;

    if (!path || !result_inode)
        return -1;

    if (path[0] == '\0')
        return -1;

    /* Determine start inode */
    if (path[0] == '/') {
        current = ROOT_INODE;
        strncpy(buf, path + 1, MAX_PATH_LEN);
    } else {
        current = cwd_inode;
        strncpy(buf, path, MAX_PATH_LEN);
    }

    buf[MAX_PATH_LEN - 1] = '\0';

    token = strtok_r(buf, "/", &saveptr);

    while (token) {
        /* Skip empty or "." */
        if (strcmp(token, "") == 0 || strcmp(token, ".") == 0) {
            token = strtok_r(NULL, "/", &saveptr);
            continue;
        }

        /* Handle ".." */
        if (strcmp(token, "..") == 0) {
            if (current != ROOT_INODE) {
                dir_entry_t e;
                if (directory_lookup(current, "..", &e) < 0)
                    return -1;
                current = e.inode;
            }
            token = strtok_r(NULL, "/", &saveptr);
            continue;
        }

        /* Normal path component */
        dir_entry_t e;
        if (directory_lookup(current, token, &e) < 0)
            return -1;

        current = e.inode;
        token = strtok_r(NULL, "/", &saveptr);
    }

    *result_inode = current;
    return 0;
}

int path_parent(const char *path,
                uint32_t cwd_inode,
                uint32_t *parent_inode,
                char *name)
{
    char buf[MAX_PATH_LEN];
    char *slash;

    if (!path || !parent_inode || !name)
        return -1;

    if (path[0] == '\0')
        return -1;

    strncpy(buf, path, MAX_PATH_LEN);
    buf[MAX_PATH_LEN - 1] = '\0';

    /* Trim trailing slash (except "/") */
    size_t len = strlen(buf);
    while (len > 1 && buf[len - 1] == '/') {
        buf[len - 1] = '\0';
        len--;
    }

    slash = strrchr(buf, '/');

    /* Case: no slash → current directory */
    if (!slash) {
        *parent_inode = cwd_inode;
        strncpy(name, buf, DIR_NAME_LEN);
        name[DIR_NAME_LEN] = '\0';
        return (strlen(name) > 0) ? 0 : -1;
    }

    /* Case: parent is root */
    if (slash == buf) {
        *parent_inode = ROOT_INODE;
        strncpy(name, slash + 1, DIR_NAME_LEN);
        name[DIR_NAME_LEN] = '\0';
        return (strlen(name) > 0) ? 0 : -1;
    }

    /* General case */
    *slash = '\0';
    strncpy(name, slash + 1, DIR_NAME_LEN);
    name[DIR_NAME_LEN] = '\0';

    if (strlen(name) == 0)
        return -1;

    return path_resolve(buf, cwd_inode, parent_inode);
}
