#include "stdlib.h"

void *memcpy(void *dest, const void *src, int count) {
	int i;
	uint8_t *d = (uint8_t*)dest, *s = (uint8_t*)src;
	for (i = 0; i < count; i++) {
		d[i] = s[i];
	}
	return dest;
}

uint8_t *memset(uint8_t *dest, uint8_t val, int count) {
	int i;
	for (i = 0; i < count; i++) {
		dest[i] = val;
	}
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
