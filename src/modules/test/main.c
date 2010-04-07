#include <gc/syscall.h>
#include <gc/mem.h>
#include <gc/server.h>
#include <gm/method.h>
#include <gm/call.h>
#include <gm/call/manager.h>
#include <stdlib.h>

#define FACTOR 4

struct method_ret nulhandle(struct method_data *d) {
	if (d->blocking) c_logSvc("Handling a nothing request.", LL_STATUS);
	else c_logSvc("Handling a nothing message.", LL_STATUS);
	return mr_void();
}

struct method_ret openhandle(struct method_data *d) {
	CHKSSTR(d, 0);
	printk("test.open: "); printk(d->parameters[0].p); printk("\n");
	return mr_err(-1);
}

void thread2(void* d) {
	Server *s = srv_get(0);
	srv_addHandler(s, M_NOTHING_VVVV, nulhandle);
	srv_addHandler(s, M_OPEN_OMVV, openhandle);
	while (1) {
		c_logSvc("{2} Waiting for a request on main object...", LL_STATUS);
		srv_handle(s, HA_WAIT);
	}
}

int main() {
	c_logSvc("Hi world from unregistered test module !", LL_NOTICE);
	c_registerSvc("test");
	c_logSvc("Creating new thread...", LL_STATUS);
	thread_new(thread2, 0);
	c_logSvc("{1} Sending a test message to manager", LL_STATUS);
	c_nothing(1, 0);

	Object t = open("test");
	printk("Open 'test' : "); printk_int(t); printk("\n");

	c_logSvc("{1} Thread now sleeping...", LL_WARNING);
	while (1) thread_sleep(1000);
	return 0;
}
