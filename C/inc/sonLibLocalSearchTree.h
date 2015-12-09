#ifndef SONLIB_LOCALSEARCHTREE_H_
#define SONLIB_LOCALSEARCHTREE_H_

typedef enum {
    PATH,
    RANK,
    ROOT
} lstNodeType;

struct _stLocalSearchTree {
    stLocalSearchTree *right, *left, *parent;
    stLocalSearchTree *prevRankRoot;
    stLocalSearchTree *nextRankRoot;
    lstNodeType type;
    int n;
    int rank;
    int level;
    void *value;
    
    //Bitmaps that say whether or not each descending
    //leaf has a tree edge and whether it has a non-tree 
    //edge
};

void stLocalSearchTree_checkTree(stLocalSearchTree *node);

stLocalSearchTree *stLocalSearchTree_construct(void);

stLocalSearchTree *stLocalSearchTree_construct2(void *value);

stLocalSearchTree *stLocalSearchTree_construct3(int n, int level);

void stLocalSearchTree_merge(stLocalSearchTree *node1, stLocalSearchTree *node2);

stLocalSearchTree *stLocalSearchTree_goToLevel(stLocalSearchTree *node, int level);

void stLocalSearchTree_destruct(stLocalSearchTree *node);

stLocalSearchTree *stLocalSearchTree_findLocalRoot(stLocalSearchTree *node);

stLocalSearchTree *stLocalSearchTree_findRoot(stLocalSearchTree *node);

void stLocalSearchTree_delete(stLocalSearchTree *node);

void stLocalSearchTree_print(stLocalSearchTree *node);

//--------------------------------------------------------------------------------

struct _stStructuralForest {
    stHash *nodes;
};

void stStructuralForest_addVertex(stStructuralForest *forest, void *v);

void stStructuralForest_link(stStructuralForest *forest, void *a, void *b, int l);

stStructuralForest *stStructuralForest_construct(void);

void stStructuralForest_destruct(stStructuralForest *forest);

stLocalSearchTree *stStructuralForest_getNode(stStructuralForest *forest, void *v);

#endif
