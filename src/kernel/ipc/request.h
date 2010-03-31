#ifndef DEF_REQUEST_H
#define DEF_REQUEST_H

#include "object.h"

#define RS_PENDING 0
#define RS_PROCESSED 1
#define RS_FINISHED 2
#define RS_INTERRUPTED 3

#define PT_VOID 0
#define PT_OBJDESC 1
#define PT_LONG 2
#define PT_LONGLONG 3		//for return values
#define PT_SHM 3			//for parameters

struct request {
	struct object *obj;
	struct thread *requester;	//0 if nonblocking message
	uint32_t func, params[3], obj_close[3];		//obj_close : object descriptors to close when requests yields an answer
	struct segment_map *shm_sndr[3], *shm_rcv[3];
	int acknowledged;	// (only for blocking requests) 0 : request is pending, 1 : request is being processes, 2 : finished, 3 : interrupted
	union {
		int64_t ll;
		uint32_t n;
	} answer;
	int errcode;	//returned when function has finished
};

struct user_request {
	uint32_t func, params[3], shmsize[3];
	int isBlocking;		// 1 : blocking request, 0 : nonblocking request (message)
};

struct user_sendrequest {
	uint32_t func, a, b, c;
	uint32_t answeri;
	int64_t answerll;
	int errcode;
};

//syscalls
int request_get(int obj, uint32_t ptr, int wait);
int request_has(int obj);
void request_answer(int obj, uint32_t answer, uint32_t answer2, int errcode);		//answer2 used for long long.
int request_mapShm(int obj, uint32_t pos, int number);

int request(int obj, uint32_t rq_ptr);
int send_msg(int obj, uint32_t rq_ptr);

#endif

