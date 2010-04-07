#ifndef DEF_METHOD_H
#define DEF_METHOD_H

#define MP(r, a, b, c) (((r << 6) | (a << 4) | (b << 2) | c) << 24)

/* 			****** FORMAT FOR #define NAMES : ******
 *  	M_<method_name>_<ret><param1><param2><param3>
 *  	where ret, param1, param2 and param3 are one of the following :
 *  	- V (0) : nothing (void)
 *  	- O (1) : object descriptor
 *  	- I (2) : int
 *  	- B (2) : int used as a boolean (0 = no, 1 = yes)
 *  	- M (3) : shared memory, only for parameters
 *  	- L (3) : int64 (long long), only for return values
 */

#define M_HANDLECHECK_BIVV	(1 | MP(2, 2, 0, 0))
#define M_HANDLECHECK_BMIV	(1 | MP(2, 3, 2, 0))
/* 	Checks if object handles that method. In case BIVV, only one method is checked for.
 *  In case BMIV, the [b] methods in shared memory [a] are checked, first one not found returns false. */

#define M_NOTHING_VVVV	(2)
/*	This method does nothing, it just checks message transmission to an object. */

#include "method/manager.h"

#endif
