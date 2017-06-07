#include "bin_tree_allocator.h"
void* my_alloc(size_t size)
{
    return aligned_alloc(CACHE_LINE_SIZE, size);
}

void* my_calloc(size_t size) {
    void* tmp = my_alloc(size);
    memset(tmp, 0, size);
    return tmp;
}

void* my_realloc(void* ptr, size_t size, size_t new_size)
{
    void* _new = my_alloc(new_size);
    memcpy(_new, ptr, size);
    free(ptr);
    return _new;
}

void my_free(void *__ptr)
{
    free(__ptr);
}

void binarytree_allocator_init(BinaryTreeAllocator *allocator) {
    allocator->pages = my_alloc(ALLOCATOR_DEFAULT_PAGES * sizeof(BinaryPage*));
    for (int i = 0; i < ALLOCATOR_DEFAULT_PAGES; i++) {
        allocator->pages[i] = my_alloc(ALLOCATOR_PAGE_SIZE * sizeof(BinaryPage));
    }
    allocator->pages_memsize = ALLOCATOR_DEFAULT_PAGES;
    allocator->pages_line = 0;
    allocator->pages_pos = 0;

    allocator->stack = my_alloc(ALLOCATOR_DEFAULT_STACK_SIZE * sizeof(BinaryPage*));
    allocator->stack_memsize = ALLOCATOR_DEFAULT_STACK_SIZE;
    allocator->stack_pos = 0;

    mutex_init(&allocator->mutex);
}

BinaryPage* binarytree_allocator_alloc(BinaryTreeAllocator *allocator) {
    BinaryPage* result = NULL;
    mutex_lock(&allocator->mutex);
    if (allocator->stack_pos > 0) { // Если есть "свободный" НОД в стеке, вернуть его.
        allocator->stack_pos--;
        result = allocator->stack[allocator->stack_pos];
    } else {
        allocator->pages_pos++;
        if (allocator->pages_pos == ALLOCATOR_PAGE_SIZE) {
            allocator->pages_pos = 0;
            allocator->pages_line++;

            if (allocator->pages_line == allocator->pages_memsize) {
                size_t memory = allocator->pages_memsize;
                size_t new_memory = memory * GOLDEN_RATIO;

                allocator->pages = my_realloc(allocator->pages, memory * sizeof(BinaryPage*), new_memory * sizeof(BinaryPage*));
                allocator->pages_memsize = new_memory;

                if (allocator->pages == NULL) {
                    printf("Allocator: не достаточно памяти\n");
                    fflush(stdout);
                    exit(1);
                }

                for (size_t i = memory; i < new_memory; i++) {
                    allocator->pages[i] = my_alloc(ALLOCATOR_PAGE_SIZE * sizeof(BinaryPage));
                }
            }
        }
        result = &(allocator->pages[allocator->pages_line][allocator->pages_pos]);
    }
    memset(result, 0, sizeof(BinaryPage));
    memory_barrier();
    mutex_unlock(&allocator->mutex);
    return result;
}

void binarytree_allocator_zero_all(BinaryTreeAllocator *allocator) {
    allocator->pages_line = 0;
    allocator->pages_pos = 0;
    allocator->stack_pos = 0;
    mutex_init(&allocator->mutex);
}

void binarytree_allocator_free_all(BinaryTreeAllocator *allocator) {
    for (size_t i = 0; i < allocator->pages_line; i++) {
        my_free(allocator->pages[i]);
    }
    my_free(allocator->pages);
    my_free(allocator->stack);
}
