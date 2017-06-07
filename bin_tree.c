#include <bin_tree.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "config.h"
#include "ministl.h"

static void binarytree_balance_in_cluster(BinaryPage *ptr, Key key, Value value);

bool is_key(int bfs_index)
{
    return bfs_index < KEYS_COUNT;
}

bool is_next(int bfs_index)
{
    return !(is_key(bfs_index));
}

bool is_exist(BinaryPage *ptr, uint pos)
{
    return ((ptr->exist[pos >> 3]) & pow2(pos & 0x7));
}

void set_exist(BinaryPage *ptr, uint pos, bool exist)
{
    ptr->exist[pos >> 3] = ((ptr->exist[pos >> 3]) & ~(pow2(pos & 0x7))) | (exist << (pos & 0x7));
}

void set_key(BinaryPage *ptr, int iterator, Key key)
{
    ptr->keys_and_next[iterator] = key;
}

Key get_key(BinaryPage *ptr, int iterator)
{
    return ptr->keys_and_next[iterator];
}

void set_value(BinaryPage *ptr, int bfs_number, Value value)
{
    ptr->values[bfs_number] = value;
}

Value get_value(BinaryPage *ptr, int bfs_number)
{
    return ptr->values[bfs_number];
}

constructor_function
static void bfs_convert_veb()
{
    Veb tree = vebnew(C_SIZE);
    for (int i = 1; i <= C_SIZE; i++) {
        bfs_convert[i - 1] = vebpos(tree, i) - 1;
    }
}
///********************************************************************************************************///
///********************************************************************************************************///
///********************************************************************************************************///
///********************************************************************************************************///
static BinaryPage* binary_page_alloc(BinaryTree* tree)
{
    BinaryPage* tmp;
    tmp = binarytree_allocator_alloc(tree->allocator);
    mutex_init(&tmp->mutex);
    return tmp;
}

void binarytree_init(BinaryTree* tree)
{
    memset(tree, 0, sizeof(BinaryTree));
    tree->allocator = malloc(sizeof(BinaryTreeAllocator));
    binarytree_allocator_init(tree->allocator);
    tree->root = binary_page_alloc(tree);
    mutex_init(&tree->root->mutex);
    seqlock_init(&tree->seqlock);
}

void binarytree_reset(BinaryTree* tree) {
    binarytree_allocator_zero_all(tree->allocator);
    tree->root = binary_page_alloc(tree);
    mutex_init(&tree->root->mutex);
    seqlock_init(&tree->seqlock);
}


static bool binarytree_lookup_single_thread(BinaryTree* tree, Key key, uint8_t *bfs, BinaryPage** page)
{
    if (!bfs || !page) {
        return false;
    }

    uint8_t bfs_number, iterator, i;
    BinaryPage *ptr = tree->root;

    for (;;)
    {
        bfs_number = 0;
        iterator = BFS_CONVERT(bfs_number);

        for (i = 0; i < HEIGHT - 1; i++)
        {
            if ( !is_exist(ptr, iterator) )
            {
                *page = ptr;
                *bfs = bfs_number;
                return false;
            }

            if (get_key(ptr, iterator) > key)
            {
                bfs_number = BFS_LEFT(bfs_number);
            }
            else if (get_key(ptr, iterator) < key)
            {
                bfs_number = BFS_RIGHT(bfs_number);
            }
            else // found
            {
                *page = ptr;
                *bfs = bfs_number;
                return true;
            }

            iterator = BFS_CONVERT(bfs_number);
        }

        if ( !is_exist(ptr, iterator) )
        {
            *page = ptr;
            *bfs = bfs_number;
            return false;
        }
        ptr = (BinaryPage*)ptr->keys_and_next[iterator];
    }
}


bool binarytree_insert_single_thread(BinaryTree* tree, Key key, Value value)
{
    uint8_t bfs_number = 0;
    BinaryPage* ptr = NULL;

    if (binarytree_lookup_single_thread(tree, key, &bfs_number, &ptr))
    {
        return false;
    }

    uint8_t iterator = BFS_CONVERT(bfs_number);

    if ( is_next(bfs_number) ) // сортировка кластерная
    {
        if (!ptr->was_sorted && ptr->size < KEYS_COUNT)
        {
            binarytree_balance_in_cluster(ptr, key, value);
            return true;
        }
        else
        {
            ptr->was_sorted = true;
            ptr->keys_and_next[iterator] = (uintptr_t) binary_page_alloc(tree);
            set_exist(ptr, iterator, true);
            ptr = (BinaryPage*)ptr->keys_and_next[iterator];
            bfs_number = 0;
            iterator = BFS_CONVERT(bfs_number);
        }
    }

    ptr->size++;
    set_key(ptr, iterator, key);
    set_value(ptr, bfs_number, value);
    set_exist(ptr, iterator, true);
    return true;
}

bool binarytree_search_single_thread(BinaryTree *tree, Key key, Value **value)
{
    uint8_t bfs_number = 0;
    BinaryPage* ptr = NULL;

    bool found = binarytree_lookup_single_thread(tree, key, &bfs_number, &ptr);

    if (!found)
    {
        value = NULL;
        return false;
    }

    *value = &ptr->values[bfs_number];
    return true;
}


/*
bool binarytree_lookup(BinaryTree* tree, Key key, uint8_t *bfs, BinaryPage** page)
{
    if (!bfs || !page) {
        return false;
    }

    uint8_t bfs_number;
    uint8_t iterator;
    int8_t i;

    mutex_lock(&tree->mutex);
    mutex_lock(&tree->root->mutex);

    BinaryPage *ptr = tree->root;
    BinaryPage *next = NULL;
    read_memory_barrier();
    mutex_unlock(&tree->mutex);
    for (;;)
    {
        bfs_number = 0;
        iterator = BFS_CONVERT(bfs_number);
        for (i = 0; i < HEIGHT - 1; i++)
        {
            if ( !is_exist(ptr, iterator) )
            {
                *page = ptr;
                *bfs = bfs_number;
                return false;
            }

            if (key < get_key(ptr, iterator))
            {
                bfs_number = BFS_LEFT(bfs_number);
            }
            else if (key > get_key(ptr, iterator))
            {
                bfs_number = BFS_RIGHT(bfs_number);
            }
            else
            {
                *page = ptr;
                *bfs = bfs_number;
                return true;
            }
            iterator = BFS_CONVERT(bfs_number);
        }

        if ( !is_exist(ptr, iterator) )
        {
            *page = ptr;
            *bfs = bfs_number;
            return false;
        }

        next = (BinaryPage*)ptr->keys_and_next[iterator];
        log("next = %p", next);
        memory_barrier();
        mutex_lock(&next->mutex);
        mutex_unlock(&ptr->mutex);
        ptr = next;
    }
}
*/
// Нужен стек/список, чтобы организовать откат
// указатели, на указатели...

#define COMMAND_RESTART  0x00000001
#define COMMAND_INSERT   0x00000010
#define set_insert(x) x |= COMMAND_INSERT
#define set_restart(x) x |= COMMAND_RESTART
#define is_restart(x) (!(x ^ COMMAND_RESTART))
#define is_insert(x)  (!(x ^ COMMAND_INSERT))

bool binarytree_insert(BinaryTree* tree, Key key, Value value)
{
    uint8_t bfs_number = 0;
    uint8_t iterator;
    int i;


    // будем считать, что ptr->root всегда есть и не освобождается при удалениях.
    BinaryPage *ptr = tree->root;
    BinaryPage **prev = &tree->root;
    BinaryPage **prev_next = &tree->root;
    uint32_t seq = 0;
    for (;;) {
        int32_t boolean = 0;
        seq = seqlock_read_begin(&ptr->seqlock);
        bfs_number = 0;
        iterator = BFS_CONVERT(bfs_number);

        for (i = 0; i < HEIGHT - 1; i++) {
            if ( !is_exist(ptr, iterator) )
            {
                if (seqlock_read_try(&ptr->seqlock, seq) && seqlock_write_try(&ptr->seqlock)) {
                    set_insert(boolean);
                    break;
                }
                set_restart(boolean);
                break;
                // иначе мы вылетаем наверх и снова становимся ридерами
            }

            if (key < get_key(ptr, iterator)) {
                bfs_number = BFS_LEFT(bfs_number);
            } else if (key > get_key(ptr, iterator)) {
                bfs_number = BFS_RIGHT(bfs_number);
            } else { // Элемент есть в дереве.
                if (seqlock_read_try(&ptr->seqlock, seq)) {
                    return false;
                }
                set_restart(boolean);
                break;
            }
            iterator = BFS_CONVERT(bfs_number);
        }
        if (is_insert(boolean)) {
            break;
        }

        if (is_restart(boolean)) {
            ptr = *prev;
            continue;
        }

        if ( !is_exist(ptr, iterator) )
        {
            if (seqlock_read_try(&ptr->seqlock, seq) && seqlock_write_try(&ptr->seqlock)) {
                break;
            } else {
                ptr = *prev;
                continue;
            }
        }
        // либо помнить указатель, по которому пришли, либо откатываться на целую структуру назад.
        prev_next = &(ptr->keys_and_next[iterator]);
        if (seqlock_read_try(&ptr->seqlock, seq)) {
            ptr = *prev_next;
            prev = prev_next;
        } else {
            // наверх
            ptr = *prev;
        }
    }

    if ( is_next(bfs_number) && (!ptr->was_sorted && ptr->size < KEYS_COUNT)) {
        binarytree_balance_in_cluster(ptr, key, value);
    } else {
        if ( is_next(bfs_number) ) {
            ptr->was_sorted = true;
            ptr->keys_and_next[iterator] = (uintptr_t) binary_page_alloc(tree);
            set_exist(ptr, iterator, true);
            BinaryPage* ptr_next = (BinaryPage*)ptr->keys_and_next[iterator];

            seqlock_write_lock(&ptr_next->seqlock);
            memory_barrier();
            seqlock_write_unlock(&ptr->seqlock);

            ptr = ptr_next;
            bfs_number = 0;
            iterator = BFS_CONVERT(bfs_number);
        }
        ptr->size++;
        set_key(ptr, iterator, key);
        set_value(ptr, bfs_number, value);
        set_exist(ptr, iterator, true);
    }
    memory_barrier();
    seqlock_write_unlock(&ptr->seqlock);
    return true;
}
/*
bool binarytree_search(BinaryTree *tree, Key key, Value *value)
{
    uint8_t bfs_number = 0;
    BinaryPage* ptr = NULL;

    bool found = binarytree_lookup(tree, key, &bfs_number, &ptr);

    if (!found)
    {
        *value = NULL;
    } else {
        *value = ptr->values[bfs_number];
    }
    mutex_unlock(&ptr->mutex);
    return found;
}
*/
bool binarytree_search(BinaryTree *tree, Key key, Value *value)
{
    uint8_t bfs_number = 0;
    uint8_t iterator;
    int8_t i;

    // будем считать, что ptr->root всегда есть и не освобождается при удалениях.
    BinaryPage *ptr = tree->root;
    BinaryPage *ptr_next = tree->root;
    BinaryPage **prev = &tree->root;
    BinaryPage **prev_next = &tree->root;
    uint32_t seq = 0;

    for (;;)
    {
        start:
        seq = seqlock_read_begin(&ptr->seqlock);
        bfs_number = 0;
        iterator = BFS_CONVERT(bfs_number);
        for (i = 0; i < HEIGHT - 1; i++)
        {
            if ( !is_exist(ptr, iterator) )
            {
                if (seqlock_read_try(&ptr->seqlock, seq)) {
                    return false;
                }
                ptr = *prev;
                goto start;
            }
            if (key < get_key(ptr, iterator))
            {
                bfs_number = BFS_LEFT(bfs_number);
            }
            else if (key > get_key(ptr, iterator))
            {
                bfs_number = BFS_RIGHT(bfs_number);
            }
            else // Элемент есть в дереве.
            {
                *value = ptr->values[bfs_number];
                if (seqlock_read_try(&ptr->seqlock, seq)) {
                    return true;
                }
                // вернуться "назад" и пройти маршрут заново
                ptr = *prev;
                goto start;
            }
            iterator = BFS_CONVERT(bfs_number);
        }

        if ( !is_exist(ptr, iterator) )
        {
            if (seqlock_read_try(&ptr->seqlock, seq)) {
                return false;
            }
            ptr = *prev;
            goto start;
        }
        // либо помнить указатель, по которому пришли, либо откатываться на целую структуру назад.
        prev_next = &(ptr->keys_and_next[iterator]);
        ptr_next = *prev_next;

        if (seqlock_read_try(&ptr->seqlock, seq)) {
            ptr = ptr_next;
            prev = prev_next;
        } else {
            // наверх
            ptr = *prev;
            goto start;
        }
    }
}


/// CLUSTER SORT
#pragma pack(push,1)
typedef struct {
    Key key;
    Value value;
} element_t;

typedef struct {
    element_t data[KEYS_COUNT];
    int iterator;
} Stack;
#pragma pack(pop)

static element_t* cluster_stack_top(Stack *stack)
{
    return &stack->data[stack->iterator - 1];
}

static void cluster_stack_push(Stack *stack, Key key, Value value)
{
    stack->data[stack->iterator].key = key;
    stack->data[stack->iterator].value = value;
    stack->iterator++;
}

static void cluster_stack_pop(Stack *stack)
{
    stack->iterator--;
}

static void cluster_stack_init(Stack *stack)
{
    stack->iterator = 0;
}

static void binarytree_cluster_in_order(BinaryPage* ptr, uint8_t bfs_number, bool* insert, Key key, Value value, Stack *stack)
{
    int iterator;
    start:

    iterator = BFS_CONVERT(bfs_number);

    if (is_exist(ptr, BFS_LEFT_CONVERT(bfs_number)) && is_key( BFS_LEFT(bfs_number) ))
    {
        binarytree_cluster_in_order(ptr, BFS_LEFT(bfs_number), insert, key, value, stack);
    }

    if (get_key(ptr,iterator) > key && !(*insert))
    {
        cluster_stack_push(stack, key, value);
        *insert = true;
    }

    cluster_stack_push(stack, get_key(ptr, iterator), get_value(ptr, bfs_number));
    set_exist(ptr, iterator, false);

    if (is_exist(ptr, BFS_RIGHT_CONVERT(bfs_number)) && is_key( BFS_RIGHT(bfs_number) ))
    {
        bfs_number = BFS_RIGHT(bfs_number);
        goto start;
    }
}

static void binarytree_insert_rec(BinaryPage* ptr, int left, int right, uint8_t bfs_number, Stack *stack)
{
    int mid;
    uint8_t iterator;

    start:

    mid = (left + right) / 2;
    if (left > right)
    {
        return;
    }

    iterator = BFS_CONVERT(bfs_number);

    set_key(ptr, iterator, stack->data[mid].key);
    set_value(ptr, bfs_number, stack->data[mid].value);
    set_exist(ptr, iterator, true);

    binarytree_insert_rec(ptr, left, mid - 1, BFS_LEFT(bfs_number), stack);

    left = mid + 1;
    bfs_number = BFS_RIGHT(bfs_number);
    goto start;
}

static void binarytree_balance_in_cluster(BinaryPage *ptr, Key key, Value value)
{
    Stack stack;
    cluster_stack_init(&stack);
    bool insert = false;
    binarytree_cluster_in_order(ptr, 0, &insert, key, value, &stack);
    // если новый ключ - наибольший
    if (!insert)
    {
        cluster_stack_push(&stack, key, value);
    }
    ptr->size++;
    binarytree_insert_rec(ptr, 0, stack.iterator-1, 0, &stack);

}

bool binarytree_delete(BinaryTree* tree, Key key)
{
    return false;
}

void binarytree_shutdown(BinaryTree* tree)
{
    binarytree_allocator_free_all(tree->allocator);
    free(tree->allocator);
}
