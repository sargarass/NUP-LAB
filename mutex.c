#include "mutex.h"
#define WAIT_MODULO 0xFFFF
#define WAIT_CIRCLE(wait_cond) \
    for (uint64_t i = 1; (wait_cond); i++) { \
        if (!(i & WAIT_MODULO)) { \
            thread_wait(); \
        } \
    }

void mutex_init(Mutex *mutex) {
    mutex->mute = 0;
}

bool mutex_try_lock(Mutex *mutex) {
    if (mutex->mute) {
        return false;
    }
    int8_t used = 1;
    asm volatile
    ("xchg %1, %0 \n"
    : "+m"(mutex->mute), "+r"(used)
    :
    :
    );

    return (used == 0);
}

void mutex_lock(Mutex *mutex) {
    for(;;) {
        WAIT_CIRCLE(mutex->mute);
        if ( mutex_try_lock(mutex) ) {
            break;
        }
    }
}

void mutex_unlock(Mutex *mutex) {
    mutex->mute = 0;
    write_memory_barrier();
    log("mutex[%p] unlocked\n", mutex);
}



/*log("try to lock mutex[%p]\n", mutex);
#if defined (__GNUC__)
do {
    WAIT_CIRCLE(mutex->mute);
    used = 1;
    asm volatile
    ("xchg al, [rdi] \n"
    : "+a"(used)
    : "D"(&mutex->mute)
    :
    );
} while(used == 1);
#elif defined(_MSC_VER)
__asm {
    push edi
    push eax
    ; Аргумент функции sleep - 0
    mov eax, 0
    push eax

    mov edi, mutex
    jmp yy
xx:
    call sleep ; pause
yy:
    movzx	eax, byte ptr [edi].mute
    test	al, al
    jne	xx
    mov al, 1;
    xchg al, [edi];
    and al, al;
    jne xx

    pop eax
    pop eax
    pop edi
}
#endif
log("mutex[%p] locked\n", mutex);

    #elif defined(_MSC_VER)
    __asm {
        push eax
        push edi

        mov edi, mutex
        mov al, 0
        xchg al, [edi].mute

        pop edi
        pop eax
    }
    #endif
*/
