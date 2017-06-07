#ifndef MINISDL_H
#define MINISDL_H
#include "include.h"
#include "limits.h"

#ifdef DEBUG
#define exec_dg(x) x
#else
#define exec_dg(x)
#endif

#define is_null(ptr)  \
{  \
if (ptr == NULL)  \
    {  \
        printf("[ERROR] Pointer at %s:%llu is NULL \n",__FILE__,__LINE__); \
        exit(1);  \
    }  \
}

#define CREATE_VECTOR_TYPE_H(type)  \
typedef struct vector_##type vector_##type; \
typedef struct vector_##type { \
    size_t size;  \
    size_t size_mem; \
    type* array; \
    void (*push_back)(vector_##type* vec, const type elem); \
    size_t (*length)(vector_##type* vec); \
    void (*resize)(vector_##type* vec, const size_t size); \
    void (*set)(vector_##type* vec, const size_t pos, type element); \
    type* (*get)(vector_##type* vec, const size_t pos); \
    void (*show)(vector_##type* vec,const char* specifier, bool debug); \
    void (*clear)(vector_##type* vec); \
    void (*delete)(vector_##type* vec); \
}; \
\
void __vector_delete_##type(vector_##type* vec) \
{ \
    if (vec->array) \
        free(vec->array); \
 \
    vec->size_mem = 0; \
    vec->size = 0; \
    vec->array = NULL; \
} \
 \
void __vector_clear_##type(vector_##type* vec) \
{ \
    memset(vec->array, 0, sizeof(type)*(vec->size_mem)); \
} \
 \
void __vector_push_back_##type(vector_##type* vec, type element) \
{ \
    if (vec->size >= vec->size_mem) \
    { \
        vec->array = realloc(vec->array, sizeof(type) * (vec->size_mem + 1) * GOLDEN_RATIO); \
        is_null(vec->array); \
        memset(vec->array + vec->size_mem, 0 , ((vec->size_mem + 1) * GOLDEN_RATIO - vec->size_mem)*sizeof(type)); \
         \
    vec->size_mem = (vec->size_mem + 1) * GOLDEN_RATIO; \
    } \
    vec->array[vec->size++] = element; \
} \
 \
void __vector_resize_##type(vector_##type* vec, const size_t size) \
{ \
    if (size > vec->size_mem) \
    { \
        vec->array = realloc(vec->array, sizeof(type) * (size + 1) * GOLDEN_RATIO); \
        is_null(vec->array); \
        memset(vec->array + vec->size_mem, 0 , ((size + 1) * GOLDEN_RATIO - vec->size_mem)*sizeof(type)); \
        vec->size_mem = (size + 1) * GOLDEN_RATIO; \
    } \
    vec->size = size; \
} \
 \
void __vector_show_##type(vector_##type* vec, const char* specifier, bool debug) \
{ \
    int i; \
    size_t n = debug? vec->size_mem : vec->size; \
    for (i = 0; i < n; i++) \
    { \
        printf(specifier,vec->array[i]); \
    } \
    printf(" \n"); \
} \
 \
size_t __vector_length_##type(vector_##type* vec) \
{ \
    return vec->size; \
} \
 \
type* __vector_get_##type(vector_##type* vec, const size_t pos) \
{ \
    if (pos < vec->size) \
        return &vec->array[pos]; \
 \
    exec_dg(printf("[WARNING] Vector.get() %p: is out_of_range \n",vec)); \
    exit(-1); \
    return NULL; \
} \
 \
void __vector_set_##type(vector_##type* vec, const size_t pos, type element) \
{ \
    if (pos < vec->size) \
    { \
        vec->array[pos] = element; \
    } \
    else \
    { \
        exec_dg(printf("[WARNING] Vector.set() %p: is out_of_range \n",vec));  \
        exit(-1);  \
    } \
} \
\
vector_##type __vector_##type##_create() \
{ \
    vector_##type vec;  \
     \
    vec.size_mem = 0; \
    vec.size = 0; \
    vec.array = NULL; \
     \
    vec.push_back = __vector_push_back_##type; \
    vec.length = __vector_length_##type; \
    vec.resize = __vector_resize_##type; \
    vec.get = __vector_get_##type; \
    vec.set = __vector_set_##type; \
    vec.show = __vector_show_##type; \
    vec.clear = __vector_clear_##type; \
    vec.delete = __vector_delete_##type; \
    return vec; \
} \

#define CREATE_STACK_TYPE_H(type) \
typedef struct stack_##type stack_##type;\
typedef struct stack_##type\
{ \
    vector_##type data; \
    type (*pop)(stack_##type* this); \
    void (*push)(stack_##type* this, type item); \
    bool (*empty)(stack_##type* this); \
    size_t (*top)(stack_##type* this); \
    size_t (*cap)(stack_##type* this); \
    void (*delete)(stack_##type* this); \
}; \
\
size_t __stack_##type##_top(stack_##type* this) \
{ \
    return this->data.size - 1;\
}\
\
size_t __stack_##type##_cap(stack_##type* this)\
{\
    return this->data.size_mem;\
}\
\
type __stack_##type##_pop(stack_##type* this) \
{ \
    if (this->empty) \
    { \
        this->data.size--; \
        return this->data.array[this->data.size]; \
    } \
    else \
    { \
        exec_dg(printf("[WARNING] Stack.pop() %p: is empty! \n",this));  \
        exit(-1);  \
    } \
} \
 \
void __stack_##type##_push(stack_##type* this, type item) \
{ \
    this->data.push_back(&this->data, item); \
} \
 \
bool __stack_##type##_empty(stack_##type* this) \
{\
    return (this->data.size == 0); \
} \
\
void __stack_##type##_delete(stack_##type* this) \
{ \
    this->data.delete(&this->data); \
} \
\
stack_##type __stack_##type##_create() \
{ \
    stack_##type tmp; \
    tmp.data = __vector_##type##_create(); \
    tmp.pop = __stack_##type##_pop; \
    tmp.push = __stack_##type##_push; \
    tmp.empty = __stack_##type##_empty; \
    tmp.cap = __stack_##type##_cap; \
    tmp.top = __stack_##type##_top; \
    tmp.delete = __stack_##type##_delete; \
    return tmp; \
}

#define CREATE_QUEUE_TYPE_H(type) \
typedef struct queue_##type queue_##type; \
typedef struct queue_##type \
{ \
    size_t head; \
    size_t tail; \
    size_t count; \
    vector_##type data; \
    bool (*empty)(queue_##type* this); \
    void (*enqueue)(queue_##type* this, type item); \
    type (*dequeue)(queue_##type* this); \
    void (*delete)(queue_##type* this); \
}; \
 \
void __queue_##type##_delete(queue_##type* this) \
{ \
    this->data.delete(&this->data); \
} \
 \
bool __queue_##type##_empty(queue_##type* this) \
{ \
    return (this->count == 0); \
} \
 \
void __queue_##type##_enqueue(queue_##type* this, type item) \
{ \
    if (this->count == this->data.size) \
    { \
        size_t size = this->data.size - this->head; \
        this->data.resize(&this->data, this->data.size * 2); \
 \
        if (this->tail == this->head && this->count > 0) \
        { \
            memmove(this->data.array + this->data.size - size, this->data.array + this->head, size * sizeof(type)); \
        } \
        this->head = this->data.size - size; \
    } \
 \
    this->data.array[this->tail] = item; \
    this->tail++; \
    if (this->tail == this->data.size) \
    { \
        this->tail = 0; \
    } \
    this->count++; \
} \
 \
type __queue_##type##_dequeue(queue_##type* this) \
{ \
    if (__queue_##type##_empty(this)) \
    { \
        exec_dg(printf("[WARNING] Queue.dequeue() %p: is empty! \n", this)); \
        exit(-1); \
    } \
 \
    type tmp = this->data.array[this->head]; \
    this->head++; \
    if (this->head == this->data.size) \
    { \
        this->head = 0; \
    } \
    this->count--; \
 \
    return tmp; \
} \
 \
queue_##type __queue_##type##_create() \
{ \
    queue_##type tmp; \
    tmp.data = __vector_##type##_create(); \
    tmp.data.resize(&tmp.data, 4); \
    tmp.count = 0; \
    tmp.head = 0; \
    tmp.tail = 0; \
 \
    tmp.empty = __queue_##type##_empty; \
    tmp.dequeue = __queue_##type##_dequeue; \
    tmp.enqueue = __queue_##type##_enqueue; \
    tmp.delete = __queue_##type##_delete; \
\
    return tmp; \
}

#define INCLUDE_STL_CLASS(type) \
CREATE_VECTOR_TYPE_H(type) \
CREATE_QUEUE_TYPE_H(type) \
CREATE_STACK_TYPE_H(type)
#define constructor(type) __##type##_create()

#define INCLUDE_STL_STACK(type) \
CREATE_VECTOR_TYPE_H(type) \
CREATE_STACK_TYPE_H(type) \

#endif // MINISDL_H
