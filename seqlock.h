#ifndef SEQLOCK_H
#define SEQLOCK_H
#include "mutex.h"

typedef struct {
    int value;
    Mutex mutex;
} SeqLock;

void seqlock_init(SeqLock *seqlock);
int seqlock_read_begin(SeqLock *seqlock);
bool seqlock_read_try(SeqLock *seqlock, int value);
bool seqlock_write_try(SeqLock *seqlock);
void seqlock_write_lock(SeqLock *seqlock);
void seqlock_write_unlock(SeqLock *seqlock);
#endif // SEQLOCK_H
