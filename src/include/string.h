#ifndef _DEF_STRING_H
#define _DEF_STRING_H

#include <gc/syscall.h>
#include <stdlib.h>

void *memcpy(void *dest, const void *src, int count);
uint8_t *memset(uint8_t *dest, uint8_t val, int count);
uint16_t *memsetw(uint16_t *dest, uint16_t val, int count);

int strlen(const char *str);
char *strcpy(char *dest, const char *src);
char *strdup(const char *src);
char *strchr(const char *str, char c);
char *strcat(char *dest, const char *src);
int strcmp(const char *s1, const char *s2);

#endif
