#include <gc/syscall.h>
#include <gc/mem.h>
#include <gc/server.h>
#include <gm/method.h>

#define FACTOR 4

int obj = -1;

struct method_ret nulhandle(struct method_data *d) {
	if (d->blocking) printk("[test:2?] Handling a request.\n");
	else printk("[test:2?] Handling a message.\n");
	return mr_void();
}

void thread2(void* d) {
	printk("[test:2] Creating new object...\n");
	Server *s = srv_create();
	srv_addHandler(s, M_NOTHING_VVVV, nulhandle);
	obj = s->id;
	while (1) {
		printk("[test:2] Waiting for a request...\n");
		srv_handle(s, HA_WAIT);
	}
}

int main() {
	int i;

	printk("[test:1] Hi world !\n");
	printk("[test:1] Creating new thread...\n");
	thread_new(thread2, 0);
	while (obj == -1);
	printk("[test:1] Object was created. Sending request...\n");
	struct user_sendrequest sr;
	sr.func = M_NOTHING_VVVV;
	request(obj, &sr);
	printk("[test:1] Got answer. Now sending request to manager...\n");
	request(1, &sr);
	printk("[test:1] And now a message to manager...\n");
	send_msg(1, &sr);

	printk("[test:1] testing malloc and free...");
	int* v = malloc(10 * sizeof(int));
	if (v == 0) printk("zero");
	int* vv = malloc(10 * sizeof(int));
	for (i = 0; i < 10; i++) { v[i] = i * 1243; }
	for (i = 0; i < 10; i++) { vv[i] = i * 2; }
	for (i = 0; i < 10; i++) {
		if (v[i] != i * 1243) printk("FAIL");
	}
	for (i = 0; i < 10; i++) {
		if (vv[i] != i * 2) printk("fail");
	}
	free(v); free(vv);
	printk("nothing bad happened :)\n");

	printk("[test:1] HAHA !!! Death of [test] in 1 seconds!\n");
	thread_sleep(1000);
	return 0;
}
