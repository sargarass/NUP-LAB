#ifndef BIN_TREE_ALLOCATOR_H
#define BIN_TREE_ALLOCATOR_H
#include "include.h"
#include "config.h"
#include "bin_tree.h"
#include "mutex.h"

#define ALLOCATOR_PAGE_SIZE 65536
#define ALLOCATOR_DEFAULT_PAGES 100
#define ALLOCATOR_DEFAULT_STACK_SIZE 10000

struct BinaryTreeAllocator {
    BinaryPage **pages;
    size_t pages_line;
    size_t pages_pos;
    size_t pages_memsize;

    BinaryPage **stack;
    size_t stack_memsize;
    size_t stack_pos;

    Mutex mutex;
};

void *my_alloc(size_t __size);
void *my_realloc(void* ptr, size_t __size, size_t new_size);
void *my_calloc(size_t __size);
void my_free(void *__ptr);
void binarytree_allocator_init(BinaryTreeAllocator *allocator);
BinaryPage *binarytree_allocator_alloc(BinaryTreeAllocator *allocator);
void binarytree_allocator_free_all(BinaryTreeAllocator *allocator);
void binarytree_allocator_zero_all(BinaryTreeAllocator *allocator);
#endif // BIN_TREE_ALLOCATOR_H
