#ifndef MUTEX_H
#define MUTEX_H
#include "config.h"
#include "include.h"
#include "inttypes.h"

#pragma pack(push, 1)
typedef struct {
    volatile int8_t mute;
} Mutex;
#pragma pack(pop)

void mutex_init(Mutex *mutex);
void mutex_lock(Mutex *mutex);
void mutex_unlock(Mutex *mutex);
bool mutex_try_lock(Mutex *mutex);

#endif // MUTEX_H
