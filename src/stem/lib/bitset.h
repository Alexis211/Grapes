#ifndef DEF_BITSET_H
#define DEF_BITSET_H

#include <types.h>

#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

struct bitset {
	uint32_t *bits;
	uint32_t size;
};

void bitset_set(struct bitset* t, uint32_t num);
void bitset_clear(struct bitset* t, uint32_t num);
uint32_t bitset_test(struct bitset* t, uint32_t num);
uint32_t bitset_firstFree(struct bitset* t);

#endif
