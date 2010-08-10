#ifndef DEF_SYS_H
#define DEF_SYS_H

#include "types.h"

void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);

#define PANIC(s) panic(s, __FILE__, __LINE__);
#define ASSERT(s) { if (!(s)) panic_assert(#s, __FILE__, __LINE__); }
void panic(char* message, char* file, int line);
void panic_assert(char* assertion, char* file, int line);

/*	For some actions, we use cli and sti as a global system mutex.
	These actions include : system initialization, process creation, interrupt handling. */
void cli();	//lock
void sti();	//unlock

#endif
