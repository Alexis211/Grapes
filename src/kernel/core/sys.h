#ifndef DEF_SYS_H
#define DEF_SYS_H

#include "types.h"

void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);

#define PANIC(s) panic(s, __FILE__, __LINE__);
void panic(char* message, char* file, int line);

void sti();		//GLOBAL SYSTEM MUTEX
void cli();

#define WHERE { monitor_write("(kernel:"); monitor_write(__FILE__); monitor_write(":"); monitor_writeDec(__LINE__); monitor_write(") "); }

#endif
