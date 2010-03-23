#ifndef DEF_OBJECT_H
#define DEF_OBJECT_H

#include <task/task.h>

struct object {
	struct process *owner;	//when 0, object is invalid and cannot handle requests
	int descriptors;
	uint32_t busyMutex;			//if busy, either a blocking request is being processed, or a sent message is waiting for being recieved
	struct request *request;
};

struct obj_descriptor {
	struct object *obj;
	int id;
	struct obj_descriptor *next;
};

//Objects
struct object* obj_new(struct process *owner);
void obj_delete(struct object* obj);

int obj_createP(struct process* p);
void obj_closeP(struct process* p, int id);
void obj_closeall(struct process* p);

//Object descriptors
int objdesc_add(struct process* proc, struct object* obj);		// add a descriptor
int objdesc_get(struct process* proc, struct object* obj);		// look in descriptors for the one corresponding to the object
struct object* objdesc_read(struct process* proc, int id);		// get the object correspoinding to the id
void objdesc_rm(struct process* proc, int id);					// remove descriptor for an object

//Syscalls
int object_create();
int object_owned(int id);		//does current process own object ? 1=yes 0=no
void object_close(int id);		//closes descriptor to specified object. if we are the owner, make all requests to object fail.

#endif

