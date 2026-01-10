#ifndef DISK_H
#define DISK_H

#include <stdint.h>

#define BLOCK_SIZE 4096

int disk_open(const char *path);
int disk_read(uint32_t block_no, void *buffer);
int disk_write(uint32_t block_no, const void *buffer);
void disk_close();

#endif
