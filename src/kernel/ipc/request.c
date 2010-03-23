#include "request.h"

int request_get(int obj, uint32_t ptr, int wait) {
	//check if we own the object, if not return -2
	//check if a request is pending. if request is being processed, return -3
	//if not (busymutex unlocked or request==0) && wait, then wait, else return -1
	//if request pending, write it to ptr
	// if request is nonblocking and no shm is to be mapped, delete it and unlock objects busymutex
	//return 0
}

int request_has(int obj) {
	//check if we own the object, if not return -2
	//check if a request is pending.
	// if being processed, return 2
	// if waiting for ack, return 1
	// if none (busymutex unlocked or request==0), return 0
}

void request_answer(int obj, uint32_t answer) {
	//check if we own the object, if not return
	//if no blocking request is being processed (including non-acknowledged waiting requests), return
	//set blocking request to finished, and set its answer
	//dereference request from object, unlock objects busymutex
}

int request_mapShm(int obj, uint32_t pos, int number) {
	//check if we own the object, if not return -2
	//if no request is being processes (including non-acknowledged waiting requests), return -3
	//check if the requests has shm in parameter [number], if not return -4
	//map shm to position
	//if request is nonblocking and no more shm is to be mapped, delete request and free object busymutex
	//return 0
}

static struct request *mkrequest(int obj, struct thread *requester,
		uint32_t func, uint32_t a, uint32_t b, uint32_t c) {
	// get object from descriptor id, if none return 0
	// waitlock object's busy mutex
	// if object cannot answer (owner == 0) return 0
	// create request, fill it up, reference it from object
	// return request	
}

int request(int obj, uint32_t func, uint32_t a, uint32_t b, uint32_t c, uint32_t answerptr) {
	//call mkrequest with parameters (requester thread = current thread)
	//if returned value is 0, return -1
	//sleep until request is handled
	//write answer to *answerptr, delete request, return 0
}

int send_msg(int obj, uint32_t func, uint32_t a, uint32_t b, uint32_t c) {
	//call mkrequest with parameters (requester thread = 0)
	//if returned value is 0, return -1 else return 0
}
