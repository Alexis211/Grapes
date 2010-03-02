#ifndef DEF_REQUEST_H
#define DEF_REQUEST_H

#include "object.h"

#define A_STILLRUNNING 0
#define A_NUMBER 1
#define A_OBJDESCRIPTOR 2
#define A_VOID 3

struct request {
	struct object *obj;
	struct thread *requester;
	uint32_t func, param1, param2, param3;
	struct seg_map *shm_cli, *shm_srv;
	int answerType;
	union {
		int num;
		struct object* obj;
	} answer;
};

struct user_request {
	uint32_t func, param1, param2, param3;
	int hasShm;
};

#endif

