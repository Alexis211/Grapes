#include <gc/syscall.h>

#define FACTOR 4

int obj = -1;

void thread2(void* d) {
	printk("[test:2] Creating new object...\n");
	obj = object_create();
	struct user_request rq;
	while (1) {
		printk("[test:2] Waiting for a request...\n");
		request_get(obj, &rq, 1);
		if (rq.isBlocking) {
			printk("[test:2] Got request. Answering...\n");
			request_answer(obj, 42, 0);
		} else {
			printk("[test:2] Got message. Ignoring it.\n");
		}
	}
}

int main() {
	printk("[test:1] Hi world !\n");
	printk("[test:1] Creating new thread...\n");
	thread_new(thread2, 0);
	while (obj == -1);
	printk("[test:1] Object was created. Sending request...\n");
	struct user_sendrequest sr;
	sr.func = 0x80000001;
	request(obj, &sr);
	printk("[test:1] Got answer. Sending message...\n");
	send_msg(obj, &sr);
	printk("[test:1] HAHA !!! Death in 10 seconds!\n");
	thread_sleep(10000);
	return 0;
}
