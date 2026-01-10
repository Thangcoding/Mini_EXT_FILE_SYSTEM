#ifndef DIR_ENTRY_H
#define DIR_ENTRY_H

#include <stdint.h>

#define DIR_NAME_LEN 255 

/* File type definition */
#define FILE_TYPE_REGULAR   0
#define FILE_TYPE_DIRECTORY 1

/*
 * dir_entry_t
 * Represents one entry inside a directory
 */
typedef struct dir_entry {
    uint32_t inode;        // inode number
    uint16_t rec_len;      // size of this directory entry
    uint8_t  name_len;     // length of name
    uint8_t  file_type;    // 0 = file, 1 = directory
    char     name[DIR_NAME_LEN + 1]; // null-terminated
} dir_entry_t;

#endif
