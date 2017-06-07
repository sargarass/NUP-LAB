#include "seqlock.h"

void seqlock_init(SeqLock *seqlock) {
    seqlock->value = 0;
    mutex_init(&seqlock->mutex);
}

int seqlock_read_begin(SeqLock *seqlock) {
    return seqlock->value;
}

bool seqlock_read_try(SeqLock *seqlock, int value) {
    return ((is_even(seqlock->value)) && seqlock->value == value);
}

bool seqlock_write_try(SeqLock *seqlock) {
    if (mutex_try_lock(&seqlock->mutex)) {
        seqlock->value++;
        write_memory_barrier();
        return true;
    }
    return false;
}

void seqlock_write_lock(SeqLock *seqlock) {
    mutex_lock(&seqlock->mutex);
    seqlock->value++;
    write_memory_barrier();
}

void seqlock_write_unlock(SeqLock *seqlock) {
    seqlock->value++;
    write_memory_barrier();
    mutex_unlock(&seqlock->mutex);
}

