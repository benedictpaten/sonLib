#include "sonLibGlobalsPrivate.h"

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
	return key1 - key2;
}

st_SortedSet *st_sortedSet_construct() {
	return st_sortedSet_construct3(st_sortedSet_cmpFn, NULL);
}

st_SortedSet *st_sortedSet_construct2(void (*destructElementFn)(void *)) {
	return st_sortedSet_construct3(st_sortedSet_cmpFn, destructElementFn);
}

static int st_sortedSet_construct3P(const void *a, const void *b, int (*cmpFn)(const void *, const void *)) {
	return cmpFn(a, b);
}

st_SortedSet *st_sortedSet_construct3(int (*compareFn)(const void *, const void *),
									  void (*destructElementFn)(void *)) {
	st_SortedSet *sortedSet = st_malloc(sizeof(st_SortedSet));
	sortedSet->sortedSet = avl_create((int (*)(const void *, const void *, void *))st_sortedSet_construct3P, compareFn, NULL);
	sortedSet->destructElementFn = destructElementFn;
	return sortedSet;
}

static void (*st_sortedSet_destruct_destructElementFn)(void *);
static void st_sortedSet_destructP(void *a, void *b) {
	assert(b != NULL);
	st_sortedSet_destruct_destructElementFn(a);
}

void st_sortedSet_destruct(st_SortedSet *sortedSet) {
	if(sortedSet->destructElementFn != NULL) {
		st_sortedSet_destruct_destructElementFn = sortedSet->destructElementFn;
		avl_destroy(sortedSet->sortedSet, (void (*)(void *, void *))st_sortedSet_destructP);
	}
	else {
		avl_destroy(sortedSet->sortedSet, NULL);
	}
}

void st_sortedSet_insert(st_SortedSet *sortedSet, void *object) {
	avl_insert(sortedSet->sortedSet, object);
}

void *st_sortedSet_find(st_SortedSet *sortedSet, void *object) {
	return avl_find(sortedSet->sortedSet, object);
}

void st_sortedSet_delete(st_SortedSet *sortedSet, void *object) {
	avl_delete(sortedSet->sortedSet, object);
}

int32_t st_sortedSet_getLength(st_SortedSet *sortedSet) {
	return avl_count(sortedSet->sortedSet);
}

void *st_sortedSet_getFirst(st_SortedSet *items) {
	static st_SortedSetIterator iterator;
	avl_t_init(&iterator, items->sortedSet);
	return avl_t_first(&iterator, items->sortedSet);
}

void *st_sortedSet_getLast(st_SortedSet *items) {
	static st_SortedSetIterator iterator;
	avl_t_init(&iterator, items->sortedSet);
	return avl_t_last(&iterator, items->sortedSet);
}

st_SortedSetIterator *st_sortedSet_getIterator(st_SortedSet *items) {
	st_SortedSetIterator *iterator;
	iterator = st_malloc(sizeof(st_SortedSetIterator));
	avl_t_init(iterator, items->sortedSet);
	return iterator;
}

void st_sortedSet_destructIterator(st_SortedSetIterator *iterator) {
	free(iterator);
}

void *st_sortedSet_getNext(st_SortedSetIterator *iterator) {
	return avl_t_next(iterator);
}

st_SortedSetIterator *st_sortedSet_copyIterator(st_SortedSetIterator *iterator) {
	st_SortedSetIterator *copyIterator;
	copyIterator = st_malloc(sizeof(st_SortedSetIterator));
	avl_t_copy(copyIterator, iterator);
	return copyIterator;
}

void *st_sortedSet_getPrevious(st_SortedSetIterator *iterator) {
	return avl_t_prev(iterator);
}
