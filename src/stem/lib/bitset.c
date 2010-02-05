#include "bitset.h"

void bitset_set(struct bitset* t, uint32_t num) {
	uint32_t idx = INDEX_FROM_BIT(num);
	uint32_t off = OFFSET_FROM_BIT(num);
	t->bits[idx] |= (0x1 << off);
}

void bitset_clear(struct bitset* t, uint32_t num) {
	uint32_t idx = INDEX_FROM_BIT(num);
	uint32_t off = OFFSET_FROM_BIT(num);
	t->bits[idx] &= ~(0x1 << off);
}

uint32_t bitset_test(struct bitset* t, uint32_t num) {
	uint32_t idx = INDEX_FROM_BIT(num);
	uint32_t off = OFFSET_FROM_BIT(num);
	return (t->bits[idx] & (0x1 << off));
}

uint32_t bitset_firstFree(struct bitset* t) {
	uint32_t i, j;
	for (i = 0; i < INDEX_FROM_BIT(t->size); i++) {
		if (t->bits[i] != 0xFFFFFFFF) {
			for (j = 0; j < 32; j++) {
				uint32_t toTest = 0x1 << j;
				if (!(t->bits[i] & toTest)) {
					return i*4*8+j;
				}
			}
		}
	}
	return (uint32_t) - 1;
}

