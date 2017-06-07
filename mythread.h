#ifndef MYTHREAD_H
#define MYTHREAD_H
#include "config.h"

typedef struct {
    void* Thread_ptr;
} Thread;

void thread_create(Thread *thread, void* (*func)(void*), void* args);
void thread_join(Thread *thread);
void thread_free(Thread *thread);
#endif // MYTHREAD_H
