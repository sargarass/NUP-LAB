#ifndef RW_LOCK_H
#define RW_LOCK_H
#include "config.h"
#include "include.h"

typedef struct {
   volatile int32_t lock;
   volatile int32_t writer_wait;
} RWLock;

void rwlock_init(RWLock *rwlock);
void rwlock_read_lock(RWLock *rwlock);
void rwlock_write_lock(RWLock *rwlock);
bool rwlock_try_read_lock(RWLock *rwlock);
bool rwlock_try_write_lock(RWLock *rwlock);
void rwlock_write_unlock(RWLock *rwlock);
void rwlock_read_unlock(RWLock *rwlock);
#endif // RW_LOCK_H
