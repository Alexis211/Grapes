#ifndef DEF_MONITOR_H
#define DEF_MONITOR_H

#include "types.h"

void monitor_put(char c);
void monitor_clear();
void monitor_write(char *s);
void monitor_writeHex(uint32_t v);
void monitor_writeDec(uint32_t v);

#define NL monitor_put('\n');

#endif

