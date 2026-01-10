#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>

/* bitmap operations */
int bitmap_test(uint32_t bitmap_block, uint32_t index, uint32_t max_bits);
int bitmap_set(uint32_t bitmap_block, uint32_t index, uint32_t max_bits);
int bitmap_clear(uint32_t bitmap_block, uint32_t index, uint32_t max_bits);

/* allocation helpers */
int bitmap_alloc(uint32_t bitmap_block,
                 uint32_t start,
                 uint32_t max_bits);

#endif
