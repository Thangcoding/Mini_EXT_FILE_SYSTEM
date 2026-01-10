#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

#define BLOCK_SIZE        4096
#define TOTAL_BLOCKS      1024       
#define TOTAL_INODES      128
#define INODE_SIZE        128

static int disk_fd = -1;

int disk_open(const char *path)
{
    disk_fd = open(path, O_RDWR);
    if (disk_fd < 0) {
        perror("disk_open");
        return -1;
    }
    return 0;
}

int disk_read(uint32_t block_no, void *buffer)
{   
    if (disk_fd < 0) return -1;
    if (block_no >= TOTAL_BLOCKS) return -1;
    off_t offset = (off_t)block_no * BLOCK_SIZE;
    if (lseek(disk_fd, offset, SEEK_SET) < 0)
        return -1;

    return read(disk_fd, buffer, BLOCK_SIZE);
}

int disk_write(uint32_t block_no, const void *buffer)
{
    off_t offset = (off_t)block_no * BLOCK_SIZE;
    if (lseek(disk_fd, offset, SEEK_SET) < 0)
        return -1;

    return write(disk_fd, buffer, BLOCK_SIZE);
}

void disk_close()
{
    if (disk_fd >= 0)
        close(disk_fd);
}
