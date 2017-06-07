#ifndef CONFIG_H
#define CONFIG_H

//#define DEBUG
#define GOLDEN_RATIO 1.61803398875
#define pow2(n) (1<<(n))

#define is_odd(n) ((n) & 1)
#define is_even(n) (!is_odd(n))

#define inline __attribute__((always_inline))
#if defined (__GNUC__)
#define __ALIGN(n) __attribute__((aligned(n)))
#define constructor_function __attribute__((constructor))
#define memory_barrier() asm volatile("mfence" ::: "memory")
#define read_memory_barrier() asm volatile ("lfence" ::: "memory")
#define write_memory_barrier() asm volatile ("sfence" ::: "memory")
#define barrier() asm volatile ("")
#define thread_wait() sched_yield()
#elif defined(_MSC_VER)
#define __ALIGN(n) __declspec(align(n))
#define memory_barrier() asm{ mfence }
#define read_memory_barrier() asm{ lfence }
#define write_memory_barrier() asm{ sfence }
#define barrier() asm{}
#define thread_wait() sleep(0)
#else
#error "Unsupported compiler"
#endif

#if defined (DEBUG)
#define log(format, args...) printf(format, ##args)
#else
#define log(format, ...)
#endif

#endif // CONFIG_H
