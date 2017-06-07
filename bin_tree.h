#ifndef BIN_TREE_H
#define BIN_TREE_H

#include <inttypes.h>
#include "include.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include "veb.h"
#include "mutex.h"
#include "seqlock.h"
#include "rwlock.h"
#include "bin_tree_allocator.h"

#define NEXT_COUNT (((C_SIZE + 1) >> 1))
#define KEYS_COUNT ((C_SIZE) - (NEXT_COUNT))
#define EXIST_COUNT (ROUND_UP(C_SIZE, 8)/8)
#pragma pack(push, 1)

struct BinaryPage
{
    uint8_t exist[EXIST_COUNT];
    Key keys_and_next[C_SIZE];
    Value values[KEYS_COUNT];
    //Mutex mutexes[C_SIZE];

    uint8_t size;
    uint8_t was_sorted;

    SeqLock seqlock;
    RWLock rwlock;
    Mutex mutex;
} ;

struct BinaryTree
{
    BinaryPage *root;

    SeqLock seqlock;
    RWLock rwlock;
    Mutex mutex;

    BinaryTreeAllocator *allocator;
};

#pragma pack(pop)
void binarytree_reset(BinaryTree* tree);
void binarytree_init(BinaryTree *tree);
void binarytree_shutdown(BinaryTree* tree);
bool binarytree_insert(BinaryTree *tree, Key key, Value value);
bool binarytree_delete(BinaryTree *tree, Key key);
bool binarytree_search(BinaryTree *tree, Key key, Value *value);
void binarytree_inorder(BinaryTree *tree, void (*func)(BinaryPage *page, int iterator, int left_son, int right_son));
void binarytree_graphviz_output(BinaryTree *tree, FILE *out);

bool is_key(int bfs_index);
bool is_next(int bfs_index);
bool is_exist(BinaryPage *ptr, uint pos);
void set_exist(BinaryPage *ptr, uint pos, bool exist);
void set_key(BinaryPage *ptr, int iterator, Key key);
void set_value(BinaryPage *ptr, int bfs_number, Value value);
Key get_key(BinaryPage *ptr, int iterator);
Value get_value(BinaryPage *ptr, int bfs_number);

static uint8_t bfs_convert[C_SIZE];
#define BFS_CONVERT(x) (bfs_convert[x])
#define BFS_RIGHT_CONVERT(x) (BFS_CONVERT(BFS_RIGHT(x)))
#define BFS_LEFT_CONVERT(x)  (BFS_CONVERT(BFS_LEFT(x)))
#define INORDER_CONVERT_BFS(height, x) (inorder_convert_bfs[height][x])
#define INORDER_CONVERT(height, x) (BFS_CONVERT(INORDER_CONVERT_BFS(height,x)))

#endif
