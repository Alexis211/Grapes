#ifndef DEF_OBJECT_H
#define DEF_OBJECT_H

#include <task/task.h>

#define OS_INACTIVE  0		//No one doing anything on this
#define OS_LISTENING 1		//A thread is waiting for a request on this object
#define OS_REQUESTPENDING 2	//A request is waiting for a thread to handle it
#define OS_BUSY	3			//A thread is currently handling a request

struct object {
	struct process *owner;
	int descriptors;
	uint32_t busyMutex;
	struct request *request;
};

struct obj_descriptor {
	struct object *obj;
	int id;
	int owned;
	struct obj_descriptor *next;
};

#endif

