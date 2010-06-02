#include "sonLibGlobalsInternal.h"
#include "avl.h"

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//Functions on a sorted set and its iterator
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct _stSortedSet {
    struct avl_table *sortedSet;
    void (*destructElementFn)(void *);
};

static int st_sortedSet_cmpFn( const void *key1, const void *key2 ) {
    return key1 > key2 ? 1 : key1 < key2 ? -1 : 0;
}

stSortedSet *stSortedSet_construct() {
    return stSortedSet_construct3(st_sortedSet_cmpFn, NULL);
}

stSortedSet *stSortedSet_construct2(void (*destructElementFn)(void *)) {
    return stSortedSet_construct3(st_sortedSet_cmpFn, destructElementFn);
}

struct _stSortedSet_construct3Fn {
    int (*compareFn)(const void *, const void *);
};

static int st_sortedSet_construct3P(const void *a, const void *b, struct _stSortedSet_construct3Fn *c) { //int (*cmpFn)(const void *, const void *)) {
    return c->compareFn(a, b);
}

stSortedSet *stSortedSet_construct3(int (*compareFn)(const void *, const void *),
                                      void (*destructElementFn)(void *)) {
    stSortedSet *sortedSet = st_malloc(sizeof(stSortedSet));
    struct _stSortedSet_construct3Fn *i = st_malloc(sizeof(struct _stSortedSet_construct3Fn));
    i->compareFn = compareFn; //this is a total hack to make the function pass ISO C compatible.
    sortedSet->sortedSet = avl_create((int (*)(const void *, const void *, void *))st_sortedSet_construct3P, i, NULL);
    sortedSet->destructElementFn = destructElementFn;
    return sortedSet;
}

static void (*st_sortedSet_destruct_destructElementFn)(void *);
static void st_sortedSet_destructP(void *a, void *b) {
    assert(b != NULL);
    st_sortedSet_destruct_destructElementFn(a);
}

void stSortedSet_destruct(stSortedSet *sortedSet) {
    if(sortedSet->destructElementFn != NULL) {
        st_sortedSet_destruct_destructElementFn = sortedSet->destructElementFn;
        avl_destroy(sortedSet->sortedSet, (void (*)(void *, void *))st_sortedSet_destructP);
    }
    else {
        free(sortedSet->sortedSet->avl_param); //clean up the param..
        avl_destroy(sortedSet->sortedSet, NULL);
    }
}

void stSortedSet_insert(stSortedSet *sortedSet, void *object) {
    avl_insert(sortedSet->sortedSet, object);
}

void *stSortedSet_search(stSortedSet *sortedSet, void *object) {
    return avl_find(sortedSet->sortedSet, object);
}

void stSortedSet_remove(stSortedSet *sortedSet, void *object) {
    avl_delete(sortedSet->sortedSet, object);
}

int32_t stSortedSet_size(stSortedSet *sortedSet) {
    return avl_count(sortedSet->sortedSet);
}

void *stSortedSet_getFirst(stSortedSet *items) {
    static stSortedSetIterator iterator;
    avl_t_init(&iterator, items->sortedSet);
    return avl_t_first(&iterator, items->sortedSet);
}

void *stSortedSet_getLast(stSortedSet *items) {
    static stSortedSetIterator iterator;
    avl_t_init(&iterator, items->sortedSet);
    return avl_t_last(&iterator, items->sortedSet);
}

stSortedSetIterator *stSortedSet_getIterator(stSortedSet *items) {
    stSortedSetIterator *iterator;
    iterator = st_malloc(sizeof(stSortedSetIterator));
    avl_t_init(iterator, items->sortedSet);
    return iterator;
}

void stSortedSet_destructIterator(stSortedSetIterator *iterator) {
    free(iterator);
}

void *stSortedSet_getNext(stSortedSetIterator *iterator) {
    return avl_t_next(iterator);
}

stSortedSetIterator *stSortedSet_copyIterator(stSortedSetIterator *iterator) {
    stSortedSetIterator *copyIterator;
    copyIterator = st_malloc(sizeof(stSortedSetIterator));
    avl_t_copy(copyIterator, iterator);
    return copyIterator;
}

void *stSortedSet_getPrevious(stSortedSetIterator *iterator) {
    return avl_t_prev(iterator);
}
