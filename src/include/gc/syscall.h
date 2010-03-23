#ifndef DEF_SYSCALL_H
#define DEF_SYSCALL_H

typedef unsigned size_t;

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

#endif
