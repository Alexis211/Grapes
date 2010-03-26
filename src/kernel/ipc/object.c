#include "object.h"
#include <lib/mutex.h>
#include <mem/mem.h>

struct object* obj_new(struct process* owner) {
	struct object* ret = kmalloc(sizeof(struct object));
	ret->owner = owner;
	ret->descriptors = 0;
	ret->busyMutex = MUTEX_UNLOCKED;
	ret->request = 0;
	return ret;
}

void obj_delete(struct object* obj) {
	if (obj->descriptors > 0) return;
	if (obj->busyMutex != MUTEX_UNLOCKED) return;
	if (obj->request != 0) return;
	kfree(obj);
}

int obj_createP(struct process* p) {
	return objdesc_add(p, obj_new(p));
}

void obj_closeP(struct process* p, int id) {
	struct object* obj = objdesc_read(p, id);
	if (obj == 0) return;
	objdesc_rm(p, id);
	if (obj->owner == p) {
		if (obj->descriptors > 0) {	//TODO !!!
			obj->owner = 0; // set object to be invalid
			//if a request was being handled, set it to interrupted (acknowledged = 3) and wake up receiver thread or if nonblocking delete it
			//unlock objects busymutex
		} else {
			obj_delete(obj);
		}
	} else {
		if (obj->descriptors == 0 && obj->owner == 0) {
			obj_delete(obj);
		} else if (obj->descriptors == 1 && obj->owner != 0) {
			//future : send message becuz object closed for everyone
		}
	}
}

void obj_closeall(struct process* p) {
	while (p->objects != 0) obj_closeP(p, p->objects->id);
}

// DESCRIPTORS

int objdesc_add(struct process* proc, struct object* obj) {
	int tmp = objdesc_get(proc, obj);
	if (tmp != -1) { return -1; }	//signal that a descriptor already exists
	struct obj_descriptor *ret = kmalloc(sizeof(struct obj_descriptor));
	ret->obj = obj;
	ret->id = proc->next_objdesc;
	ret->next = proc->objects;
	proc->objects = ret;
	obj->descriptors++;
	proc->next_objdesc++;
	return ret->id;
}

int objdesc_get(struct process* proc, struct object* obj) {
	struct obj_descriptor *it = proc->objects;
	while (it != 0) {
		if (it->obj == obj) return it->id;
		it = it->next;
	}
	return -1;
}

struct object* objdesc_read(struct process* proc, int id) {
	struct obj_descriptor *it = proc->objects;
	while (it != 0) {
		if (it->id == id) return it->obj;
		it = it->next;
	}
	return 0;
}

void objdesc_rm(struct process* proc, int id) {
	struct obj_descriptor *e = proc->objects;
	if (e != 0 && e->id == id) {
		proc->objects = e->next;
		e->obj->descriptors--;
		kfree(e);
		return;
	}
	while (e->next != 0) {
		if (e->next->id == id) {
			e->next = e->next->next;
			e->next->obj->descriptors--;
			kfree(e->next);
			return;
		}
		e = e->next;
	}
}

// SYSCALLS

int object_create() {
	return obj_createP(current_thread->process);
}

int object_owned(int id) {
	struct object *obj = objdesc_read(current_thread->process, id);
	if (obj == 0) return -10;
	if (obj->owner == current_thread->process) return 1;
	return 0;
}

void object_close(int id) {
	obj_closeP(current_thread->process, id);
}
