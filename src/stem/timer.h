#ifndef DEF_TIMER_H
#define DEF_TIMER_H

#include "types.h"

void timer_init(uint32_t frequency);
uint32_t timer_time();		//Returns miliseconds (approximate) since computer started
uint32_t timer_uptime();	//Same thing in seconds

#endif
