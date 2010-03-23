#ifndef DEF_REQUEST_H
#define DEF_REQUEST_H

#include "object.h"

#define A_STILLRUNNING 0
#define A_NUMBER 1
#define A_OBJDESCRIPTOR 2
#define A_VOID 3

struct request {
	struct object *obj;
	struct thread *requester;	//0 if nonblocking message
	uint32_t func, params[3];
	struct seg_map *shm_srv[3], *shm_cli[3];
	union {
		int64_t ll;
		uint32_t n;
	} answer;
};

struct user_request {
	uint32_t func, param1, param2, param3;
	int hasShm;
};

//syscalls
int request_get(int obj, uint32_t ptr, int wait);
int request_has(int obj);
void request_answer(int obj, uint32_t answer);
int request_mapShm(int obj, uint32_t pos, int number);

int request(int obj, uint32_t func, uint32_t a, uint32_t b, uint32_t c, uint32_t answerptr);
int send_msg(int obj, uint32_t func, uint32_t a, uint32_t b, uint32_t c);

#endif

