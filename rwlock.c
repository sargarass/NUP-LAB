#include "rwlock.h"

/* переменная lock - 32 бита
 * 1) 24-битовый счетчик, показывающий количество потоков,
 * одновременно читающих защищаемую структуру. Значения счетчика в
 * дополнительном коде хранится в битах 0—23 этого поля;
 * 2) флаг "разблокировано", устанавливаемый, когда никакой поток не читает и
 * не пишет данные. Сбрасывается иначе.
 *
 * lock = Ох01000000 - spin блокировка свободна
 * lock = 0x00000000 - spin блокировка получена на запись
 * lock = 0x00ffffff - читает один поток
 * lock = 0x00fffffe - читает два потока
 * ... - аналогично. Главное - флаг "разблокировано" сброшен.
 * ==========================================================
 *                                                 сперто из книги "Ядро Linux"
 */
#define RW_SPIN_FREE 0x01000000
#define RW_SPIN_USED 0x00000000
#define WAIT_MODULO 0xFFFF
#define WAIT_CIRCLE(wait_cond) \
    for (uint64_t i = 1; (wait_cond); i++) { \
        if (!(i & WAIT_MODULO)) { \
            thread_wait(); \
        } \
    }

void rwlock_init(RWLock *rwlock) {
    rwlock->lock = 0;
    rwlock->writer_wait = false;
}


bool rwlock_try_read_lock(RWLock *rwlock) {
    if(!rwlock->lock || rwlock->writer_wait > 0) {
        return false;
    }

    asm volatile(
                  "lock dec %0"
                 : "+m"(rwlock->lock)
                 :
                 :
                 );

    if (rwlock->lock >= 0) {
        return true;
    }

    asm volatile(
                  "lock inc %0"
                 :"+m"(rwlock->lock)
                 :
                 :
                );
    return false;
}


void rwlock_read_lock(RWLock *rwlock) {
    for (;;) {
        WAIT_CIRCLE(!rwlock->lock || rwlock->writer_wait > 0);
        if (rwlock_try_read_lock(rwlock)) {
            break;
        }
    }
}

bool rwlock_try_write_lock(RWLock *rwlock) {
    if (rwlock->lock != RW_SPIN_FREE) {
        return false;
    }

    uint32_t used = RW_SPIN_USED;
    asm volatile
    (
     "xchg %1, %0 \n"
    : "+m"(rwlock->lock), "+a"(used)
    :
    :
    );
    // Захват успешен
    if (used == RW_SPIN_FREE) {
        return true;
    }
    // Захват не получился. Другие потоки могли инкрементировать/декрементировать
    // lock поэтому данное значение просто прибавим к прошлому результату
    asm volatile
    (
      "lock add %0, %1"
     : "+m"(rwlock->lock)
     : "r"(used)
     :
     );
    return false;
}

void rwlock_write_lock(RWLock *rwlock) {
    asm volatile
    ("lock inc %0"
    :"+m"(rwlock->writer_wait)
    :
    :
    );

    for(;;) {
        WAIT_CIRCLE(rwlock->lock != RW_SPIN_FREE);
        if (rwlock_try_write_lock(rwlock)) {
            break;
        }
    }

    asm volatile
    ("lock dec %0"
    :"+m"(rwlock->writer_wait)
    :
    :
    );

}

void rwlock_write_unlock(RWLock *rwlock) {
    asm volatile
    ("lock add %0, %1" // или всё-таки or?
    : "+m"(rwlock->lock)
    : "r"(RW_SPIN_FREE)
    :
    );
}

void rwlock_read_unlock(RWLock *rwlock) {
    asm volatile
    (
      "lock inc %0"
     :"+m"(rwlock->lock)
     :
     :
    );
}
