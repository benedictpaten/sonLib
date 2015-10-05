#include "sonLibGlobalsInternal.h"

typedef void *(*tree_function)(void* node);

void *stAbstractTree_first(tree_function parent, tree_function left, void *node) {
    while(parent(node)) {
        node = parent(node);
    }
    while(left(node)) {
        node = left(node);
    }
    return node;
}

void *stAbstractTree_inOrderNext(tree_function parent, tree_function left, tree_function right, void *node) {
    if (right(node)) {
        return stAbstractTree_first(parent, left, node);
    }
    
    void *p = parent(node);
    while(p && node == right(p)) {
        node = p;
        p = parent(p);
    }
    return p;
}


void stAbstractTree_print(tree_function inOrderNext, tree_function value, void *tree) {
}
