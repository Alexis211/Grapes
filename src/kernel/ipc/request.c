#include "request.h"
#include "shm.h"
#include "object.h"
#include <lib/mutex.h>
#include <mem/seg.h>
#include <mem/mem.h>

int request_get(int id, uint32_t ptr, int wait) {
	int i;
	//check if we own the object, if not return -2 (-10 if descriptor does not exist)
	struct object *obj = objdesc_read(current_thread->process, id);
	if (obj == 0) return -10;
	if (obj->owner != current_thread->process) return -2;	
	//check if a request is pending. if request is being processed (acknowledged), return -3
	if (obj->request != 0 && obj->request->acknowledged != RS_PENDING) return -3;
	//if not (busymutex unlocked and request==0) && wait, then wait, else return -1
	if (wait == 0 && obj->request == 0) return -1;
	while (obj->busyMutex != MUTEX_LOCKED && (obj->request == 0 || obj->request->acknowledged != RS_PENDING)) thread_sleep(1);
	obj->request->acknowledged = RS_PROCESSED;
	//when request pending (wait finished), write it to ptr
	struct user_request *p = (struct user_request*)ptr;
	p->func = obj->request->func;
	for (i = 0; i < 3; i++) {
		p->params[i] = obj->request->params[i];
		if (obj->request->shm_sndr[i] != 0) p->shmsize[i] = obj->request->shm_sndr[i]->len;
		else p->shmsize[i] = 0;
	}
	p->isBlocking = (obj->request->requester != 0);
	//if request is nonblocking and no shm is to be mapped, delete request and unlock objects busymutex, else set it to acknowledged
	if (p->isBlocking) return 0;
	for (i = 0; i < 3; i++) {
		if (obj->request->shm_sndr[i] != 0) return 0;
	}
	kfree(obj->request);
	obj->request = 0;
	mutex_unlock(&obj->busyMutex);
	return 0;
}

int request_has(int id) {
	//check if we own the object, if not return -2 (-10 if descriptor does not exist)
	struct object *obj = objdesc_read(current_thread->process, id);
	if (obj == 0) return -10;
	if (obj->owner != current_thread->process) return -2;	
	//check if a request is pending.
	// if none (busymutex unlocked or request==0), return 0
	if (obj->request == 0 || obj->busyMutex == MUTEX_UNLOCKED) return 0;
	// if waiting for ack (not acknowledged), return 1
	if (obj->request->acknowledged == RS_PENDING) return 1;
	// if being processed (acknowledged), return 2 
	return 2;
}

void request_answer(int id, uint32_t answer, uint32_t answer2, int errcode) {
	int i;
	//check if we own the object, if not return (also return if descriptor does not exist)
	struct object *obj = objdesc_read(current_thread->process, id);
	if (obj == 0) return;
	if (obj->owner != current_thread->process) return;	
	//if no blocking request is being processed (including non-acknowledged waiting requests), return
	if (obj->request == 0 || obj->request->acknowledged == RS_PENDING || obj->request->requester == 0) return;
	//unmap shared memory segments from shm_rcv, close descriptors to objects from obj_close
	for (i = 0; i < 3; i++) {
		if (obj->request->shm_rcv[i] != 0) seg_unmap(obj->request->shm_rcv[i]);
		if (obj->request->obj_close[i] != 0) obj_closeP(obj->owner, obj->request->obj_close[i]);
	}
	//set blocking request to finished (acknowledged = 2), and set its answer
	obj->request->acknowledged = RS_FINISHED;
	switch (obj->request->func >> 30) {
		case PT_OBJDESC:
			if ((int)answer <= 0) {
				obj->request->answer.n = answer;
			} else {
				if (obj->owner == obj->request->requester->process) {
					obj->request->answer.n = answer;
				} else {
					int n = objdesc_get(obj->request->requester->process, objdesc_read(obj->owner, answer));
					if (n == -1) {
						n = objdesc_add(obj->request->requester->process, objdesc_read(obj->owner, answer));
					}
					obj->request->answer.n = n;
				}
			}
			break;
		case PT_LONG:
			obj->request->answer.n = answer;
			break;
		case PT_LONGLONG:
			obj->request->answer.ll = (uint64_t)((uint64_t)answer2 << 32) | answer;
	}
	obj->request->errcode = errcode;
	//wake up receiver thread (thread_wakeUp)
	thread_wakeUp(obj->request->requester);
	//dereference request from object, unlock objects busymutex
	obj->request = 0;
	mutex_unlock(&obj->busyMutex);
}

int request_mapShm(int id, uint32_t pos, int number) {
	int i;
	if (number > 2 || number < 0) return -9;
	//check if we own the object, if not return -2 (-10 if descriptor does not exist)
	struct object *obj = objdesc_read(current_thread->process, id);
	if (obj == 0) return -10;
	if (obj->owner != current_thread->process) return -2;	
	//if no request is being processes (including non-acknowledged waiting requests), return -3
	if (obj->request == 0 || obj->request->acknowledged == RS_PENDING) return -3;
	//check if the requests should have shm in parameter [number], if not return -4
	int n = (obj->request->func >> (28 - (2 * number))) & 3;
	if (n != PT_SHM) return -4;
	//check if sender process is different from receiver process, if not return -7
	if (obj->owner == obj->request->requester->process) return -7;
	//check if sender sent a shm seg in parameter [number], if not return -5
	if (obj->request->shm_sndr[number] == 0) return -5;
	//map shm to position
	obj->request->shm_rcv[number] = seg_map(obj->request->shm_sndr[number]->seg, obj->owner->pagedir, pos);
	//if request is nonblocking and no more shm is to be mapped, delete request and free object busymutex
	if (obj->request->requester != 0) return 0;
	for (i = 0; i < 3; i++) {
		if (obj->request->shm_sndr[i] != 0 && obj->request->shm_rcv[i] == 0) return 0;
	}
	kfree(obj->request);
	obj->request = 0;
	mutex_unlock(&obj->busyMutex);
	return 0;
}

static struct request *mkrequest(int id, struct thread *requester,
		uint32_t func, uint32_t a, uint32_t b, uint32_t c, uint32_t *err) {
	int i;
	// get object from descriptor id, if none return 0
	struct object *obj = objdesc_read(current_thread->process, id);
	if (obj == 0) {
		*err = -10;
		return 0;
	}
	// waitlock object's busy mutex
	mutex_lock(&obj->busyMutex);
	// if object cannot answer (owner == 0) return 0
	if (obj->owner == 0) {
		mutex_unlock(&obj->busyMutex);
		*err = -11;
		return 0;
	}
	// create request, fill it up :
	struct request *rq = kmalloc(sizeof(struct request));
	rq->obj = obj;
	rq->requester = requester;
	rq->func = func;
	for (i = 0; i < 3; i++) { rq->params[i] = 0; rq->obj_close[i] = 0; rq->shm_sndr[i] = 0; rq->shm_rcv[i] = 0; }
	rq->acknowledged = RS_PENDING;
	//  integers: use as is
	//  objects: open a new descriptor in receiver process (if same process, keep number), put that number as an int
	//   if receiver already had descriptor to this object, use it and set obj_close to 0, else set obj_close to new number
	//  shm: if same process, put ptr as int, else put 0 as int and get segment_map to shm_sndr
	//  objects and shm: 0 means sender didn't want to share anything, that should stay 0.
	for (i = 0; i < 3; i++) {
		int n = (rq->func >> (28 - (2 * i))) & 3;
		uint32_t v = (i == 0 ? a : (i == 1 ? b : c));
		switch (n) {
			case PT_OBJDESC:
				if ((int)v <= 0) {
					rq->params[i] = v;
				} else {
					if (obj->owner == current_thread->process) {
						rq->params[i] = v;
					} else {
						int d = objdesc_get(obj->owner, objdesc_read(current_thread->process, v));
						if (d == -1) {
							d = objdesc_add(obj->owner, objdesc_read(current_thread->process, v));
							rq->obj_close[i] = d;
						}
						rq->params[i] = d;
					}
				}
				break;
			case PT_LONG:
				rq->params[i] = v;
				break;
			case PT_SHM:
				if (obj->owner == current_thread->process) {
					rq->params[i] = v;
				} else {
					rq->shm_sndr[i] = shmseg_getByOff(current_thread->process, v);
				}
				break;
		}
	}
	// reference request from object
	obj->request = rq;
	// return request	
	return rq;
}

int request(int obj, uint32_t rq_ptr) {
	uint32_t e = 0;

	struct user_sendrequest *urq = (void*)rq_ptr;
	//call mkrequest with parameters (requester thread = current thread)
	struct request *rq = mkrequest(obj, current_thread, urq->func, urq->a, urq->b, urq->c, &e);
	//if returned value is 0 (could not create request), return -1
	if (e != 0) return e;
	if (rq == 0) return -1;
	//sleep until request is handled
	thread_goInactive();
	//if request has been interrupted because process closed communication (acknowledged == 3), return -2
	if (rq->acknowledged == 3) return -2;
	//write answer to urq, delete request, return 0
	switch (urq->func >> 30) {
		case PT_OBJDESC:
		case PT_LONG:
			urq->answeri = rq->answer.n;
			break;
		case PT_LONGLONG:
			urq->answerll = rq->answer.ll;
	}
	urq->errcode = rq->errcode;
	kfree(rq);
	return 0;
}

int send_msg(int obj, uint32_t rq_ptr) {
	uint32_t e = 0;

	struct user_sendrequest *urq = (void*)rq_ptr;
	//call mkrequest with parameters (requester thread = 0)
	struct request *rq = mkrequest(obj, 0, urq->func, urq->a, urq->b, urq->c, &e);
	//if returned value is 0, return -1 else return 0
	if (e != 0) return e;
	if (rq == 0) return -1;
	return 0;
}
