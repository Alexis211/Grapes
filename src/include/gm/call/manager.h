#ifndef DEF_CALL_MANAGER_H
#define DEF_CALL_MANAGER_H

#include <gm/call.h>
#include <gc/obj.h>

Object c_open(_CHP, char *c);
Object open(char *c);	//calls c_open with object 1

void c_registerSvc(char *name);	//automatically calls with objecct id 0

void c_logSvc(char *log, int level);

#endif
