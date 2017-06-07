#ifndef TIMER_H
#define TIMER_H
#include <inttypes.h>
#include <time.h>

typedef struct {
    struct timespec start;
    struct timespec end;
} Timer;

void timer_start(Timer *timer);
void timer_stop(Timer *timer);
long double timer_elapsed(Timer *timer);
void timer_show(Timer *timer);
#endif // TIMER_H
