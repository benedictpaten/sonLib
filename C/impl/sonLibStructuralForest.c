#include "sonLibGlobalsInternal.h"

stStructuralForest *stStructuralForest_construct() {
    stStructuralForest *forest = st_malloc(sizeof(stStructuralForest));
    forest->nodes = stHash_construct();
    return forest;
}

void stStructuralForest_merge(stStructuralForest *forest, void *parent, void *child) {
    stLocalSearchTree *parentNode = stHash_search(forest->nodes, parent);
    stLocalSearchTree *childNode = stHash_search(forest->nodes, child);
    stLocalSearchTree_merge(parentNode, childNode);
    stHash_insert(forest->nodes, parent, stLocalSearchTree_findRoot(parentNode));

}

void stStructuralForest_deleteChild(stStructuralForest *forest, void *child) {
    stLocalSearchTree_delete(stHash_search(forest->nodes, child));
}
