#include <gc/syscall.h>
#include <gc/mem.h>
#include <gc/server.h>
#include <gm/method.h>
#include <gm/call/manager.h>
#include <string.h>

char *loglevels[] = {"Critical", "Error", "Warning", "Notice", "Status", "Debug"};
int loglevel = LL_DEBUG;

struct service {
	char* name;
	int pid;
	Object obj;
	struct service *next;
};

struct service *services = 0;

//************************ FUNCTIONS THAT DO STUFF ********************
void logsvc(int pid, char* log, int level) {
	if (level > loglevel || level < 0) return;
	struct service *svc = services;
	while (svc) {
		if (svc->pid == pid) break;
		svc = svc->next;
	}
	if (svc == 0) {
		printk("[log:??] ");
	} else {
		printk("[log:"); printk(svc->name); printk("] ");
	}
	printk(loglevels[level]); printk(": ");
	printk(log); printk("\n");
}

void registersvc(int pid, char* name, Object descriptor) {
	struct service *svc = malloc(sizeof(struct service));
	svc->name = strdup(name);
	svc->pid = pid;
	svc->obj = descriptor;
	svc->next = services;
	services = svc;
	char msg[100] = "Service registered: ";
	strcat(msg, svc->name);
	logsvc(1, msg, LL_NOTICE);
}

//******************************   HANDLING METHODS ON MAIN OBJECT	*****************
struct method_ret handle_nothing(struct method_data *d) {
	if (d->blocking) logsvc(1, "Received a {nothing} request.", LL_DEBUG);
	else logsvc(1, "Received a {nothing} message.", LL_DEBUG);
	return mr_void();
}

struct method_ret handle_open(struct method_data *d) {
	CHKSSTR(d, 0);
	char *svc = d->parameters[0].p;
	printk("open: "); printk(svc); printk("\n");
	char *sep, *res;
	sep = strchr(svc, ':');
	if (sep != 0) *sep = '0';
	struct service *s = services;
	while (s) {
		if (strcmp(s->name, svc) == 0) break;
		s = s->next;
	}
	if (s == 0) return mr_err(-2);
	if (sep == 0) {
		return mr_obj(s->obj);
	} else {
		*sep = ':';
		res = sep + 1;
		// open service svc, ressource res
		Object r = c_open(s->obj, 1, res);
		if (r == -1) return mr_err(-3);
		return mr_obj(r);
	}
}

struct method_ret handle_registersvc(struct method_data *d) {
	CHKSSTR(d, 0);
	if (d->parameters[1].i == 0) return mr_err(-1);
	registersvc(d->pid, d->parameters[0].p, d->parameters[1].i);
	return mr_void();
}

struct method_ret handle_logsvc(struct method_data *d) {
	CHKSSTR(d, 0);
	logsvc(d->pid, d->parameters[0].p, d->parameters[1].i);
	return mr_void();
}

int main() {
	logsvc(1, "Manager module configuring...", LL_NOTICE);
	registersvc(1, "manager", 0);

	Server *mgr = srv_get(0);
	srv_addHandler(mgr, M_NOTHING_VVVV, handle_nothing);

	srv_addHandler(mgr, M_OPEN_OMVV, handle_open);
	srv_addHandler(mgr, M_REGISTERSVC_VMOV, handle_registersvc);
	srv_addHandler(mgr, M_LOGSVC_VMIV, handle_logsvc);

	logsvc(1, "Manager module configured : starting request handling.", LL_STATUS);
	srv_handle(mgr, HA_LOOP);

	printk("[manager] Manager EPIC FAIL.\n");
	return 0;
}
