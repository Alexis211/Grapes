#ifndef DEF_SYSCALL_H
#define DEF_SYSCALL_H

typedef unsigned long long	uint64_t;
typedef unsigned int		uint32_t;
typedef unsigned short		uint16_t;
typedef unsigned char		uint8_t;
typedef long long	int64_t;
typedef int 		int32_t;
typedef short		int16_t;
typedef char		int8_t;

typedef unsigned size_t;

struct user_request {
	uint32_t func, params[3];
	int isBlocking;		// 1 : blocking request, 0 : nonblocking request (message)
};

struct user_sendrequest {
	uint32_t func, a, b, c;
	uint32_t answeri;
	int64_t answerll;
	int errcode;
};

void thread_exit();
void schedule();
void thread_sleep(int time);
void process_exit(int retval);
void printk(char* str);
void thread_new(void (*entry)(void*), void *data);
void irq_wait(int number);
int proc_priv();
int shm_create(size_t offset, size_t length);
int shm_delete(size_t offset);
int object_create();
int object_owned(int descriptor);
void object_close(int descriptor);
int request_get(int descriptor, struct user_request *rq, int wait);
int request_has(int descriptor);
void request_answer(int descriptor, uint32_t answer1, uint32_t answer2, int errcode);
int request_mapShm(int descriptor, size_t offset, int number);
int request(int descriptor, struct user_sendrequest *rq);
int send_msg(int descriptor, struct user_sendrequest *rq);
int proc_setheap(size_t start, size_t end);

#endif
