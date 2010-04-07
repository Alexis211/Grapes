#ifdef DEF_METHOD_H

#define M_OPEN_OMVV (10 | MP(1, 3, 0, 0))
/*	This function opens a distant ressource. Example : open("file:Root/Public/test.txt");	*/

#define M_REGISTERSVC_VMOV (11 | MP(0, 3, 1, 0))
/*	This function registers a service.
 *	parameter 1 : service name;
 *	parameter 2 : service root object.			*/

#define M_LOGSVC_VMIV (12 | MP(0, 3, 2, 0))
/*	This parameters logs an entry for service with corresponding PID. Parameter 2 is : */
#define LL_CRITICAL 0
#define LL_ERROR 1
#define LL_WARNING 2
#define LL_NOTICE 3
#define LL_STATUS 4
#define LL_DEBUG 5

#endif
