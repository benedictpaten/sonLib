#include "sonLibGlobalsPrivate.h"

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//Functions on a sorted set and its iterator
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

st_SortedSet *st_sortedSet_construct(int32_t (*compareFn)(const void *, const void *, void *)) {
	return avl_create(compareFn, NULL, NULL);
}

void st_sortedSet_destruct(st_SortedSet *sortedSet, void (*destructElementFn)(void *, void *)) {
	avl_destroy(sortedSet, destructElementFn);
}

void st_sortedSet_insert(st_SortedSet *sortedSet, void *object) {
	avl_insert(sortedSet, object);
}

void *st_sortedSet_find(st_SortedSet *sortedSet, void *object) {
	return avl_find(sortedSet, object);
}

void st_sortedSet_delete(st_SortedSet *sortedSet, void *object) {
	avl_delete(sortedSet, object);
}

int32_t st_sortedSet_getLength(st_SortedSet *sortedSet) {
	return avl_count(sortedSet);
}

void *st_sortedSet_getFirst(st_SortedSet *items) {
	static st_SortedSetIterator iterator;
	avl_t_init(&iterator, items);
	return avl_t_first(&iterator, items);
}

void *st_sortedSet_getLast(st_SortedSet *items) {
	static st_SortedSetIterator iterator;
	avl_t_init(&iterator, items);
	return avl_t_last(&iterator, items);
}

st_SortedSetIterator *st_sortedSet_getIterator(st_SortedSet *items) {
	st_SortedSetIterator *iterator;
	iterator = mallocLocal(sizeof(st_SortedSetIterator));
	avl_t_init(iterator, items);
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
	copyIterator = mallocLocal(sizeof(st_SortedSetIterator));
	avl_t_copy(copyIterator, iterator);
	return copyIterator;
}

void *st_sortedSet_getPrevious(st_SortedSetIterator *iterator) {
	return avl_t_prev(iterator);
}
