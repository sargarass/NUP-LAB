#include "bin_tree.h"


static void binarytree_inorder_rec(BinaryPage *ptr, int bfs_number, void (*func)(BinaryPage *page, int iterator, int left_son, int right_son))
{
    int iterator = BFS_CONVERT(bfs_number);

    if (is_next(bfs_number))
    {

        if (is_exist(ptr, iterator))
        {
            ptr = (BinaryPage*)ptr->keys_and_next[iterator];
        }
        else
        {
            ptr = NULL;
        }

        bfs_number = 0;
        iterator = 0;
    }

    if (!ptr || !is_exist(ptr, iterator))
    {
        return;
    }

    binarytree_inorder_rec(ptr, BFS_LEFT(bfs_number), func);
    func(ptr, iterator, BFS_LEFT_CONVERT(bfs_number), BFS_RIGHT_CONVERT(bfs_number));
    binarytree_inorder_rec(ptr, BFS_RIGHT(bfs_number), func);
}

void binarytree_inorder(BinaryTree* tree, void (*func)(BinaryPage *page, int iterator, int left_son, int right_son))
{
    binarytree_inorder_rec(tree->root, 0, func);
}

static void binarytree_graphviz_output_rec(BinaryPage *ptr, int bfs_number, FILE *out)
{
    int iterator = BFS_CONVERT(bfs_number);

    if (is_next(bfs_number))
    {
        if (is_exist(ptr, iterator))
        {
            ptr = (BinaryPage*)ptr->keys_and_next[iterator];
        }
        else
        {
            ptr = NULL;
        }

        bfs_number = 0;
        iterator = 0;
    }

    if (!ptr || !is_exist(ptr, iterator))
    {
        return;
    }

    fprintf(out, "%" PRIu64 " -> {", ptr->keys_and_next[iterator]);
    int left_iterator = BFS_LEFT_CONVERT(bfs_number);
    if (is_exist(ptr, left_iterator))
    {
        if (is_key(BFS_LEFT(bfs_number)))
        {
            fprintf(out, "%" PRIu64 " ", ptr->keys_and_next[left_iterator]);
        }
        else
        {
            BinaryPage* left = (BinaryPage*)ptr->keys_and_next[left_iterator];
            fprintf(out, "%" PRIu64 " ", left->keys_and_next[BFS_CONVERT(0)]);
        }
    }
    int right_iterator = BFS_RIGHT_CONVERT(bfs_number);
    if (is_exist(ptr, right_iterator))
    {
        if (is_key(BFS_RIGHT(bfs_number)))
        {
            fprintf(out, "%" PRIu64 " ", ptr->keys_and_next[right_iterator]);
        }
        else
        {
            BinaryPage* right = (BinaryPage*)ptr->keys_and_next[right_iterator];
            fprintf(out, "%" PRIu64 " ", right->keys_and_next[BFS_CONVERT(0)]);
        }
    }
    fprintf(out, "}\n");

    binarytree_graphviz_output_rec(ptr, BFS_LEFT(bfs_number), out);
    binarytree_graphviz_output_rec(ptr, BFS_RIGHT(bfs_number), out);
}

void binarytree_graphviz_output(BinaryTree* tree, FILE *out)
{
    fprintf(out, "digraph BFS{\nnode [shape=circle width=0.5 style=filled]\n");
    binarytree_graphviz_output_rec(tree->root, 0, out);
    fprintf(out, "}");
}
