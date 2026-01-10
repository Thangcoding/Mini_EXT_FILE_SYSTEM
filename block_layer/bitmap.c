#include "bitmap.h"
#include "disk.h"

#include <string.h>

static int bitmap_bounds_check(uint32_t index, uint32_t max_bits)
{
    if (index >= max_bits)
        return -1;
    if (index >= BLOCK_SIZE * 8)
        return -1;
    return 0;
}

int bitmap_test(uint32_t bitmap_block, uint32_t index, uint32_t max_bits)
{
    uint8_t buffer[BLOCK_SIZE];

    if (bitmap_bounds_check(index, max_bits) < 0)
        return -1;

    if (disk_read(bitmap_block, buffer) < 0)
        return -1;

    uint32_t byte = index / 8;
    uint8_t bit   = 1 << (index % 8);

    return (buffer[byte] & bit) ? 1 : 0;
}

int bitmap_set(uint32_t bitmap_block, uint32_t index, uint32_t max_bits)
{
    uint8_t buffer[BLOCK_SIZE];

    if (bitmap_bounds_check(index, max_bits) < 0)
        return -1;

    if (disk_read(bitmap_block, buffer) < 0)
        return -1;

    uint32_t byte = index / 8;
    uint8_t bit   = 1 << (index % 8);

    if (buffer[byte] & bit)
        return -1;   /* already allocated */

    buffer[byte] |= bit;

    return (disk_write(bitmap_block, buffer) < 0) ? -1 : 0;
}

int bitmap_clear(uint32_t bitmap_block, uint32_t index, uint32_t max_bits)
{
    uint8_t buffer[BLOCK_SIZE];

    if (bitmap_bounds_check(index, max_bits) < 0)
        return -1;

    if (disk_read(bitmap_block, buffer) < 0)
        return -1;

    uint32_t byte = index / 8;
    uint8_t bit   = 1 << (index % 8);

    if ((buffer[byte] & bit) == 0)
        return -1;   /* double free */

    buffer[byte] &= ~bit;

    return (disk_write(bitmap_block, buffer) < 0) ? -1 : 0;
}

int bitmap_alloc(uint32_t bitmap_block,
                 uint32_t start,
                 uint32_t max_bits)
{
    uint8_t buffer[BLOCK_SIZE];

    if (disk_read(bitmap_block, buffer) < 0)
        return -1;

    for (uint32_t i = start; i < max_bits && i < BLOCK_SIZE * 8; i++) {
        uint32_t byte = i / 8;
        uint8_t bit   = 1 << (i % 8);

        if ((buffer[byte] & bit) == 0) {
            buffer[byte] |= bit;
            if (disk_write(bitmap_block, buffer) < 0)
                return -1;
            return i;
        }
    }
    return -1;
}
