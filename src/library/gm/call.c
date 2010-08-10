#include <gc/syscall.h>
#include <gm/call.h>
#include <gm/method.h>
#include <gc/shm.h>
#include <string.h>

int c_handleCheck(_CHP, int method) {
	struct user_sendrequest sr;
	sr.func = M_HANDLECHECK_BIVV;
	sr.a = method;
	_CHC;
	if (sr.errcode) return 0;
	return sr.answeri;
}

int c_handleCheckA(_CHP, int* methods, int number) {
	struct user_sendrequest sr;
	sr.func = M_HANDLECHECK_BMIV;
	void* ptr = shm_allocNew(sizeof(int) * number);
	memcpy(ptr, methods, sizeof(int) * number);
	sr.a = (uint32_t)methods;
	sr.b = number;
	_CHC;
	shm_freeDel(ptr);
	if (sr.errcode) return 0;
	return sr.answeri;
}

int c_nothing(_CHP) {
	struct user_sendrequest sr;
	sr.func = M_NOTHING_VVVV;
	_CHC;
	return sr.errcode;
}
