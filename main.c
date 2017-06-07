#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <malloc.h>
#include "mutex.h"
#include "timer.h"
#include "mythread.h"
#include "bin_tree.h"
#include "locale.h"
#define CPUS_NUM 8
#define TESTS_INSERT 4032000*9
#define TESTS_FIND 40320000
uint64_t THREADS;
uint64_t TEST_PER_THREAD_INSERT;
uint64_t TEST_PER_THREAD_FIND;

Key *array_insert;
Key *array_find;

typedef struct {
    int id;
    int result;
    volatile int mute;
}  ThreadArgs;

BinaryTree tree;
Mutex mute;
volatile int THREADS_END;

void insert_single_thread() {
    for (int i = 0; i < TESTS_INSERT; i++) {
        binarytree_insert_single_thread(&tree, array_insert[i], i);
    }
}

void find_signle_thread() {
    Value val;
    for (uint64_t i = 0; i < TESTS_FIND; i++) {
        binarytree_search_single_thread(&tree, array_find[i], &val);
    }
}

void *insert(void *in) {
    ThreadArgs *args = ((ThreadArgs*)in);
    while(args->mute) {};
    int id = args->id;

    int result = 0;
    for (int i = 0; i < TEST_PER_THREAD_INSERT; i++) {
        result += binarytree_insert(&tree, array_insert[args->id * TEST_PER_THREAD_INSERT + i], i);
    }
    args->result = result;

    asm volatile
    ("lock inc dword ptr [rdi]"
    :
    :"D"(&THREADS_END)
    :
    );

    return 0;
}

void *find(void *in) {
    ThreadArgs *args = ((ThreadArgs*)in);
    while(args->mute) {};
    int id = args->id;

    int result = 0;
    Value val;
    for (uint64_t i = 0; i < TEST_PER_THREAD_FIND; i++) {
        result += binarytree_search(&tree, array_find[args->id * TEST_PER_THREAD_FIND + i], &val);
    }
    args->result = result;


    asm volatile
    ("lock inc dword ptr [rdi]"
    :
    :"D"(&THREADS_END)
    :
    );

    return 0;
}

void *create_test_insert_array() {
    srand(time(NULL));
    array_insert = malloc(TESTS_INSERT * sizeof(uint64_t));
    for (uint64_t i = 0; i < (TESTS_INSERT); i++) {
        array_insert[i] = rand();
    }
    return array_insert;
}

void *create_test_find_array() {
    srand(time(NULL));
    array_find = malloc(TESTS_FIND * sizeof(uint64_t));
    for (uint64_t i = 0; i < (TESTS_FIND); i++) {
        array_find[i] = rand();
    }
    return array_find;
}

void free_test_insert_array() {
    free(array_insert);
}

void free_test_find_array() {
    free(array_find);
}

void args_init(ThreadArgs *args, int id) {
    args->id = id;
    args->mute = true;
    args->result = 0;
}

void test(void *(*func)(void *), bool out) {
    Thread threads[THREADS - 1];
    ThreadArgs args[THREADS];
    THREADS_END = 0;
    for (int i = 0; i < THREADS; i++) {
        args_init(&args[i], i);
    }

    for (int i = 0; i < THREADS - 1; i++) {
        thread_create(&threads[i], func, &args[i]);
    }

    Timer start;
    timer_start(&start);

    for (int i = 0; i < THREADS; i++) {
        args[i].mute = false;
    }

    func((void*)(&args[THREADS - 1]));

    while(THREADS_END < THREADS) {};

    timer_stop(&start);
    timer_show(&start);

//    uint64_t res = 0;
//    for (int i = 0; i < THREADS; i++) {
//        if (out) printf("%d\n", args[i].result);
//        res += args[i].result;
//    }

    for (int i = 0; i < THREADS - 1; i++) {
        thread_join(&threads[i]);

    }
    for (int i = 0; i < THREADS - 1; i++) {
        thread_free(&threads[i]);
    }
//    if (out) printf("all = %d\n", res);
}

void test_run(int __THREADS, bool out) {
    binarytree_reset(&tree);
    THREADS = __THREADS;
    TEST_PER_THREAD_INSERT = (TESTS_INSERT) / (THREADS);
    TEST_PER_THREAD_FIND =  (TESTS_FIND) / (THREADS);

    test(insert, out);
    test(find, out);
}

void test_single_thread() {
    binarytree_reset(&tree);
    Timer start;
    timer_start(&start);

    insert_single_thread();

    timer_stop(&start);
    timer_show(&start);

    timer_start(&start);

    find_signle_thread();

    timer_stop(&start);
    timer_show(&start);
}

int main(void)
{
    setlocale( LC_ALL, "" );
    binarytree_init(&tree);

    create_test_insert_array();
    create_test_find_array();

    printf("Allocating memory\n");
    test_run(CPUS_NUM, false);

    printf("Testing single thread with out any sync: \n");
    test_single_thread();
    for (int i = 2; i <= CPUS_NUM; i++) {
        printf("Threads: %d\n", i);
        test_run(i, true);
    }
    free_test_find_array();
    free_test_insert_array();
    FILE *out;
    out = fopen("out.txt", "w");
   // binarytree_graphviz_output(&tree, out);
    fclose(out);

    //free(array);

    printf("Exit\n");

    return 0;
}


