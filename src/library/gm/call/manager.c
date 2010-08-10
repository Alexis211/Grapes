#include <gc/syscall.h>
#include <gm/call/manager.h>
#include <gm/method.h>
#include <gc/shm.h>
#include <string.h>

Object c_open(_CHP, char *c) {
	struct user_sendrequest sr;
	sr.func = M_OPEN_OMVV;
	void* ptr = shm_allocNew(strlen(c) + 1);
	strcpy(ptr, c);
	sr.a = (uint32_t)ptr;
	_CHC;
	shm_freeDel(ptr);
	if (sr.errcode) return -1;
	return sr.answeri;
}

Object open(char *c) {
	return c_open(1, 1, c);
}

void c_registerSvc(char *name) {
	struct user_sendrequest sr;
	sr.func = M_REGISTERSVC_VMOV;
	void* ptr = shm_allocNew(strlen(name) + 1);
	strcpy(ptr, name);
	sr.a = (uint32_t)ptr;
	sr.b = 0;
	send_msg(1, &sr);
	shm_freeDel(ptr);
}

void c_logSvc(char *log, int level) {
	struct user_sendrequest sr;
	sr.func = M_LOGSVC_VMIV;
	void* ptr = shm_allocNew(strlen(log) + 1);
	if (ptr == 0) return;
	strcpy(ptr, log);
	sr.a = (uint32_t)ptr;
	sr.b = level;
	request(1, &sr);
	shm_freeDel(ptr);
}
