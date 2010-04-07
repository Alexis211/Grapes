#ifndef DEF_CALL_H
#define DEF_CALL_H

/*
 * This file and all files in include/call/ define prototypes to helper functions for calling methods on objects.
 */

#include <gc/obj.h>

#define _CHP Object o, int block
#define _CHC if (block) request(o, &sr); else send_msg(o, &sr);

int c_handleCheck(_CHP, int method);
int c_handleCheckA(_CHP, int* methods, int number);

int c_nothing(_CHP);

#endif
