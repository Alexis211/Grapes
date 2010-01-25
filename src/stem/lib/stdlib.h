#ifndef DEF_STDLIB_H
#define DEF_STDLIB_H

#include "../types.h"

void *memcpy(void *dest, const void *src, int count);
uint8_t *memset(uint8_t *dest, uint8_t val, int count);
uint16_t *memsetw(uint16_t *dest, uint16_t val, int count);
int strlen(const char *str);

#endif

