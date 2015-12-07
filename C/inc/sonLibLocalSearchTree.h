#ifndef SONLIB_LOCALSEARCHTREE_H_
#define SONLIB_LOCALSEARCHTREE_H_

typedef enum {
    PATH,
    RANK,
} lstNodeType;

struct _stLocalSearchTree {
    stLocalSearchTree *right, *left, *parent;
    stLocalSearchTree *prevRankRoot;
    stLocalSearchTree *nextRankRoot;
    lstNodeType type;
    int rank;
    void *value;
    
    //Bitmaps that say whether or not each descending
    //leaf has a tree edge and whether it has a non-tree 
    //edge
    int *tree;
    int *non_tree;
};

void stLocalSearchTree_checkTree(stLocalSearchTree *node);

stLocalSearchTree *stLocalSearchTree_construct(void);

stLocalSearchTree *stLocalSearchTree_construct2(int rank);

stLocalSearchTree *stLocalSearchTree_construct3(void *value);

stLocalSearchTree *stLocalSearchTree_construct4(int rank, void *value);

void stLocalSearchTree_merge(stLocalSearchTree *node1, stLocalSearchTree *node2);

void stLocalSearchTree_destruct(stLocalSearchTree *node);

stLocalSearchTree *stLocalSearchTree_findRoot(stLocalSearchTree *node);

void stLocalSearchTree_delete(stLocalSearchTree *node);

void stLocalSearchTree_print(stLocalSearchTree *node);

#endif
