#ifndef INCLUDE_H
#define INCLUDE_H
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>


typedef uint8_t byte;


#define CACHE_LINE_SIZE 64
#define HEIGHT 5
#define C_SIZE ((1 << (HEIGHT))-1)

typedef uint64_t Key;
typedef uint64_t Value;
struct BinaryTreeAllocator;
typedef struct BinaryTreeAllocator BinaryTreeAllocator;
typedef struct BinaryTree BinaryTree;
typedef struct BinaryPage BinaryPage;
int log2_64 (uint64_t value);

#define ROUND_UP(x, y) (  ((x) + ((y) - 1)) & ~((y) - 1) )
//#define ROUND_UP(x, y) ((x + y - 1)/y)
#define BFS_LEFT(x)  (2*(x) + 1)
#define BFS_RIGHT(x) (2*(x) + 2)
#define BFS_PARENT(x) ((((x) - 1)) >> 1)

#endif // INCLUDE_H
