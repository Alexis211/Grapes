#ifndef DEF_SERVER_H
#define DEF_SERVER_H

#include "syscall.h"
#include "obj.h"

//parameter/return values types
#define PT_VOID 0
#define PT_OBJDESC 1
#define PT_LONG 2
#define PT_LONGLONG 3		//for return values
#define PT_SHM 3	

struct method_data {
	struct {
		union {
			int i;
			void* p;
		} val;
		int type;
		int keepShm;		//for messages : keep shared memory segment after return or unmap ? (default : 0 = unmap)
	} parameters[3];
	uint32_t func;
	int blocking;	//1 : blocking request, 0 : message
	struct object_srv *obj;
};

struct method_ret {
	union {
		int i;
		int64_t l;
	};
	int type;
	int status;	//= error code if any
};

//helper function for creating return values
struct method_ret mr_long(int val);
struct method_ret mr_llong(int64_t val);
struct method_ret mr_obj(Object* obj);
struct method_ret mr_srv(struct object_srv* obj);
struct method_ret mr_void();
struct method_ret mr_err(int error);

typedef struct method_ret (*method_handler)(struct method_data*);

struct method_srv {
	uint32_t id;
	method_handler h;

	struct method_srv *next;
};

struct object_srv {
	int id;	//descriptor
	struct method_srv *methods;

	void *data;

	struct object_srv *next;
};

typedef struct object_srv Server;

extern Server procServer;	//corresponds to descriptor 0.

//possible actions for srv_handle
#define HA_ONCE 1	//check if requests are waiting, if so handle them
#define HA_WAIT 2	//check if requests are waiting, if so handle them, else wait for one to come and handle it
#define HA_LOOP 3	//wait for requests to come, handling them in an infinite loop

void srv_handleAll();	//check all objects once
void srv_handle(Server* o, int act);

Server *srv_create();
void srv_delete(Server *o);
void srv_addHandler(Server* o, uint32_t method, method_handler h);

#endif
