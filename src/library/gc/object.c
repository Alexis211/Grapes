#include <gc/server.h>
#include <gm/method.h>
#include <gc/mem.h>

Server procServer;
static Server* servers = 0;

static method_handler getHandler(Server *o, uint32_t m);
static struct method_ret checkIfHandles(struct method_data *d);

void objsrv_init() {
	procServer.id = 0;
	procServer.methods = 0;
	procServer.next = 0;
	procServer.data = 0;
	servers = &procServer;
	srv_addHandler(&procServer, M_HANDLECHECK_BIVV, checkIfHandles);
}

void srv_handleAll() {
	Server *i = servers;
	while (i) {
		srv_handle(i, HA_ONCE);
		i = i->next;
	}
}

void srv_handle(Server *o, int act) {
	int i;

	if (act == HA_LOOP) {
		while (1) srv_handle(o, HA_WAIT);
	} else {
		struct user_request rq;
		int v = request_get(o->id, &rq, (act == HA_WAIT));
		if (v == 0) {
			method_handler m = getHandler(o, rq.func);
			if (m == 0) {
				if (rq.isBlocking) {
					request_answer(o->id, 0, 0, ME_UNHANDLED);
				}
			} else {
				struct method_data md;
				md.func = rq.func;
				md.blocking = rq.isBlocking;
				md.obj = o;
				//get parameters
				for (i = 0; i < 3; i++) {
					md.parameters[i].type = (rq.func >> (28 - (2 * i))) & 3;
					switch (md.parameters[i].type) {
						case PT_LONG:
						case PT_OBJDESC:
							md.parameters[i].val.i = rq.params[i];
							break;
						case PT_SHM:		//all the fuss about keepShm only applies to messages.
							md.parameters[i].keepShm = 1;	//if local memory or if nothing, do not unmap it
							if (rq.params[i] == 0) {	//TODO : map shm (shm manager) !!!
								md.parameters[i].val.p = 0;
								//md.parameters[i].keepShm = 0;	//we had to map it, so mark it to be unmapped
							} else {
								md.parameters[i].val.p = (void*)rq.params[i];
							}
							break;
					}
				}
				
				struct method_ret ret = m(&md);	//Call method
				if (rq.isBlocking) {
					uint32_t a = 0, b = 0;
					if (ret.type == PT_OBJDESC || ret.type == PT_LONG) a = ret.i;
					if (ret.type == PT_LONGLONG) a = (uint32_t)ret.l, b = (uint32_t)((uint64_t)ret.l >> 32);
					request_answer(o->id, a, b, ret.status);
				} else {
					for (i = 0; i < 3; i++) {
						if (md.parameters[i].type == PT_SHM && md.parameters[i].keepShm == 0) {
							//TODO : unmap shm
						}
					}
				}
			}
		}
	}
}

void srv_addHandler(Server* o, uint32_t method, method_handler h) {
	struct method_srv *s = malloc(sizeof(struct method_srv));
	s->id = method;
	s->h = h;
	s->next = o->methods;
	o->methods = s;
}

Server *srv_create() {
	Server *s = malloc(sizeof(Server));
	s->id = object_create();
	s->methods = 0;
	s->data = 0;
	s->next = servers;
	srv_addHandler(s, M_HANDLECHECK_BIVV, checkIfHandles);
	servers = s;
	return s;
}

void srv_delete(Server* s) {
	//remove s from servers
	if (servers == s) {
		servers = s->next;
	} else {
		Server *i = servers;
		while (i->next != 0) {
			if (i->next == s) {
				i->next = s->next;
				break;
			}
			i = i->next;
		}
	}
	//close s
	object_close(s->id);
	//free methods for s
	while (s->methods != 0) {
		struct method_srv *m = s->methods->next;
		free(s->methods);
		s->methods = m;
	}
	//free s
	free(s);
}

// internal use

method_handler getHandler(Server *o, uint32_t m) {
	struct method_srv *i = o->methods;
	while (i) {
		if (i->id == m) return i->h;
		i = i->next;
	}
	return 0;
}

struct method_ret checkIfHandles(struct method_data *d) {
	if (getHandler(d->obj, d->parameters[0].val.i) == 0) {
		return mr_long(0);
	}
	return mr_long(1);
}

// ***************************** HELPER FUNCTIONS FOR RETRUN VALUES

struct method_ret mr_long(int val) {
	struct method_ret r; r.status = 0; r.type = PT_LONG; r.i = val;
	return r;
}

struct method_ret mr_llong(int64_t val) {
	struct method_ret r; r.status = 0; r.type = PT_LONGLONG; r.l = val;
	return r;
}

struct method_ret mr_obj(Object* obj) {
	struct method_ret r; r.status = 0; r.type = PT_OBJDESC; r.i = obj->id;
	return r;
}

struct method_ret mr_srv(Server* obj) {
	struct method_ret r; r.status = 0; r.type = PT_OBJDESC; r.i = obj->id;
	return r;
}

struct method_ret mr_void() {
	struct method_ret r; r.status = 0; r.type = PT_VOID;
	return r;
}

struct method_ret mr_err(int error) {
	struct method_ret r; r.status = error; r.type = PT_VOID; r.l = 0;
	return r;
}
