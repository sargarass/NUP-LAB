#include "timer.h"
#include "float.h"

void timer_start(Timer *timer) {
    clock_gettime(CLOCK_REALTIME, &timer->start);
}

void timer_stop(Timer *timer) {
    clock_gettime(CLOCK_REALTIME, &timer->end);
}

void timer_thread_start(Timer *timer) {
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &timer->start);
}

void timer_thread_stop(Timer *timer) {
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &timer->end);
}

long double timer_elapsed(Timer *timer) {
    return ((((long double)timer->end.tv_sec)+ ((long double)timer->end.tv_nsec)*1.e-9)-((long double)timer->start.tv_sec+((long double)timer->start.tv_nsec)*1.e-9));
}

void timer_show(Timer *timer) {
    printf("Time elapsed: %.*Lg\n", LDBL_DIG, timer_elapsed(timer));
}
