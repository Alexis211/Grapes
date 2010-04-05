#include <gc/syscall.h>
#include <gc/mem.h>
#include <gc/server.h>
#include <gm/method.h>

struct method_ret handle_nothing(struct method_data *d) {
	if (d->blocking) printk("[manager] Received a {nothing} request.\n");
	else printk("[manager] Received a {nothing} message.\n");
	return mr_void();
}

int main() {
	printk("[manager] Manager module configuring...\n");
	Server *mgr = srv_get(0);

	srv_addHandler(mgr, M_NOTHING_VVVV, handle_nothing);

	printk("[manager] Manager module configured : starting request handling.\n");
	srv_handle(mgr, HA_LOOP);

	printk("[manager] Manager EPIC FAIL.\n");
	return 0;
}
