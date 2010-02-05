#include "stdlib.h"

void *memcpy(void *vd, const void *vs, int count) {
	uint8_t *dest = (uint8_t*)vd, *src = (uint8_t*)vs;
	uint32_t f = count % 4, n = count / 4, i;
	const uint32_t* s = (uint32_t*)src;
	uint32_t* d = (uint32_t*)dest;
	for (i = 0; i < n; i++) {
		d[i] = s[i];
	}
	if (f != 0) {
		for (i = count - f; i < count; i++) {
			dest[i] = src[i];
		}
	}
	return vd;
}

uint8_t *memset(uint8_t *dest, uint8_t val, int count) {
	int i;
	for (i = 0; i < count; i++) {
		dest[i] = val;
	}
	return dest;
}

uint16_t *memsetw(uint16_t *dest, uint16_t val, int count) {
	int i;
	for (i = 0; i < count; i++) {
		dest[i] = val;
	}
	return dest;
}

int strlen(const char *str) {
	int i = 0;
	while (str[i++]);
	return i;
}
