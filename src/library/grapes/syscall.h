#ifndef DEF_SYSCALL_H
#define DEF_SYSCALL_H

void thread_exit();
void schedule();
void thread_sleep(int time);
void process_exit(int retval);
void printk(char* str);

#endif
