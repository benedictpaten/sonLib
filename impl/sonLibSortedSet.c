#include "sonLibGlobalsInternal.h"
#include "avl.h"

const char *SORTED_SET_EXCEPTION_ID = "SORTED_SET_EXCEPTION";

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
    void *a = sortedSet->sortedSet->avl_param;
    if(sortedSet->destructElementFn != NULL) {
        st_sortedSet_destruct_destructElementFn = sortedSet->destructElementFn;
        avl_destroy(sortedSet->sortedSet, (void (*)(void *, void *))st_sortedSet_destructP);
    }
    else {
        avl_destroy(sortedSet->sortedSet, NULL);
    }
    free(a);
    free(sortedSet);
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
    stSortedSetIterator iterator;
    avl_t_init(&iterator, items->sortedSet);
    return avl_t_first(&iterator, items->sortedSet);
}

void *stSortedSet_getLast(stSortedSet *items) {
    stSortedSetIterator iterator;
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

static struct _stSortedSet_construct3Fn *stSortedSet_getComparator(stSortedSet *sortedSet) {
    return (struct _stSortedSet_construct3Fn *)sortedSet->sortedSet->avl_param;
}

static int stSortedSet_comparatorsEqual(stSortedSet *sortedSet1, stSortedSet *sortedSet2) {
    return stSortedSet_getComparator(sortedSet1)->compareFn == stSortedSet_getComparator(sortedSet2)->compareFn;
}

int stSortedSet_equals(stSortedSet *sortedSet1, stSortedSet *sortedSet2) {
    if(stSortedSet_size(sortedSet1) != stSortedSet_size(sortedSet2)) {
        return 0;
    }
    if(!stSortedSet_comparatorsEqual(sortedSet1, sortedSet2)) {
        return 0;
    }
    int (*cmpFn)(const void *, const void *) = stSortedSet_getComparator(sortedSet1)->compareFn;

    stSortedSetIterator *it1 = stSortedSet_getIterator(sortedSet1);
    stSortedSetIterator *it2 = stSortedSet_getIterator(sortedSet2);
    void *o1 = stSortedSet_getNext(it1), *o2 = stSortedSet_getNext(it2);
    while(o1 != NULL && o2 != NULL) {
        if(cmpFn(o1, o2) != 0) {
            stSortedSet_destructIterator(it1);
            stSortedSet_destructIterator(it2);
            return 0;
        }
        o1 = stSortedSet_getNext(it1);
        o2 = stSortedSet_getNext(it2);
    }
    stSortedSet_destructIterator(it1);
    stSortedSet_destructIterator(it2);
    return 1;
}

stSortedSet *stSortedSet_getUnion(stSortedSet *sortedSet1, stSortedSet *sortedSet2) {
    if(!stSortedSet_comparatorsEqual(sortedSet1, sortedSet2)) {
        stThrowNew(SORTED_SET_EXCEPTION_ID, "Comparators are not equal for creating the union of two sorted sets!\n");
    }
    stSortedSet *sortedSet3 = stSortedSet_construct3(stSortedSet_getComparator(sortedSet1)->compareFn, NULL);

    //Add those from sortedSet1
    stSortedSetIterator *it= stSortedSet_getIterator(sortedSet1);
    void *o;
    while((o = stSortedSet_getNext(it)) != NULL) {
        stSortedSet_insert(sortedSet3, o);
    }
    stSortedSet_destructIterator(it);

    //Add those from sortedSet2
    it= stSortedSet_getIterator(sortedSet2);
    while((o = stSortedSet_getNext(it)) != NULL) {
        stSortedSet_insert(sortedSet3, o);
    }
    stSortedSet_destructIterator(it);

    return sortedSet3;
}

stSortedSet *stSortedSet_getIntersection(stSortedSet *sortedSet1, stSortedSet *sortedSet2) {
    if(!stSortedSet_comparatorsEqual(sortedSet1, sortedSet2)) {
        stThrowNew(SORTED_SET_EXCEPTION_ID, "Comparators are not equal for creating an intersection of two sorted sets!\n");
    }
    stSortedSet *sortedSet3 = stSortedSet_construct3(stSortedSet_getComparator(sortedSet1)->compareFn, NULL);

    //Add those from sortedSet1 only if they are also in sortedSet2
    stSortedSetIterator *it= stSortedSet_getIterator(sortedSet1);
    void *o;
    while((o = stSortedSet_getNext(it)) != NULL) {
        if(stSortedSet_search(sortedSet2, o) != NULL) {
            stSortedSet_insert(sortedSet3, o);
        }
    }
    stSortedSet_destructIterator(it);

    return sortedSet3;
}

stSortedSet *stSortedSet_getDifference(stSortedSet *sortedSet1, stSortedSet *sortedSet2) {
    if(!stSortedSet_comparatorsEqual(sortedSet1, sortedSet2)) {
        stThrowNew(SORTED_SET_EXCEPTION_ID, "Comparators are not equal for creating the sorted set difference !\n");
    }
    stSortedSet *sortedSet3 = stSortedSet_construct3(stSortedSet_getComparator(sortedSet1)->compareFn, NULL);

    //Add those from sortedSet1 only if they are not in sortedSet2
    stSortedSetIterator *it= stSortedSet_getIterator(sortedSet1);
    void *o;
    while((o = stSortedSet_getNext(it)) != NULL) {
        if(stSortedSet_search(sortedSet2, o) == NULL) {
            stSortedSet_insert(sortedSet3, o);
        }
    }
    stSortedSet_destructIterator(it);

    return sortedSet3;
}
