#include "sonLibGlobalsInternal.h"

#include <assert.h>

struct _stStructuralForest {
    stLocalSearchTree *tree;
    stStructuralForest *parent;
};

stStructuralForest *stStructuralForest_construct() {
    stStructuralForest *node = st_malloc(sizeof(stStructuralForest));
    node->tree = stLocalSearchTree_construct();
    return node;
}

void stStructuralForest_addChild(stStructuralForest *parent, stStructuralForest *child) {
    
}
void stStructuralForest_deleteChild(stStructuralForest *parent, stStructuralForest *child) {
}

void stStructuralForest_merge(stStructuralForest *x, stStructuralForest *y) {
}
