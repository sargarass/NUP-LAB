#include "mythread.h"
#if defined (__GNUC__)
#include <pthread.h>
void thread_create(Thread *thread, void* (*func)(void*), void* args) {
    pthread_t *handle = malloc(sizeof(pthread_t));
    thread->Thread_ptr = (void*)handle;
    pthread_create(handle, NULL, func, args);
}

void thread_join(Thread *thread)
{
    pthread_t ptr = *(pthread_t*)thread->Thread_ptr;
    pthread_join(ptr, NULL);
}

void thread_free(Thread *thread)
{
    free(thread->Thread_ptr);
}

#elif defined (_MSC_VER)
#include <windows.h>
void thread_create(Thread *thread, (void*)(func), void* args) {
    HANDLE *handle = malloc(sizeof(HANDLE));
    thread->Thread_ptr = (void*)handle;
    CreateThread(NULL, 0, func, handle, 0, NULL);
}

void thread_join(Thread *thread)
{
    HANDLE *handle = (HANDLE*)thread->Thread_ptr);
    WaitForSingleObject(*handle, INFINITE);
}

void thread_free(Thread *thread)
{
    HANDLE *handle = (HANDLE*)thread->Thread_ptr);
    CloseHandle(*handle);
    free(thread->Thread_ptr);
    thread->Thread_ptr = NULL;
}

#endif
