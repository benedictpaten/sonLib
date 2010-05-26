#include "sonLibGlobalsPrivate.h"

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//Functions on a sorted set and its iterator
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

stSortedSet *st_sortedSet_construct(int32_t (*compareFn)(const void *, const void *, void *)) {
	return avl_create(compareFn, NULL, NULL);
}

void st_sortedSet_destruct(stSortedSet *sortedSet, void (*destructElementFn)(void *, void *)) {
	avl_destroy(sortedSet, destructElementFn);
}

void st_sortedSet_insert(stSortedSet *sortedSet, void *object) {
	avl_insert(sortedSet, object);
}

void *st_sortedSet_find(stSortedSet *sortedSet, void *object) {
	return avl_find(sortedSet, object);
}

void st_sortedSet_delete(stSortedSet *sortedSet, void *object) {
	avl_delete(sortedSet, object);
}

int32_t st_sortedSet_getLength(stSortedSet *sortedSet) {
	return avl_count(sortedSet);
}

void *st_sortedSet_getFirst(stSortedSet *items) {
	static stSortedSet_Iterator iterator;
	avl_t_init(&iterator, items);
	return avl_t_first(&iterator, items);
}

void *st_sortedSet_getLast(stSortedSet *items) {
	static stSortedSet_Iterator iterator;
	avl_t_init(&iterator, items);
	return avl_t_last(&iterator, items);
}

stSortedSet_Iterator *st_sortedSet_getIterator(stSortedSet *items) {
	stSortedSet_Iterator *iterator;
	iterator = mallocLocal(sizeof(stSortedSet_Iterator));
	avl_t_init(iterator, items);
	return iterator;
}

void st_sortedSet_destructIterator(stSortedSet_Iterator *iterator) {
	free(iterator);
}

void *st_sortedSet_getNext(stSortedSet_Iterator *iterator) {
	return avl_t_next(iterator);
}

stSortedSet_Iterator *st_sortedSet_copyIterator(stSortedSet_Iterator *iterator) {
	stSortedSet_Iterator *copyIterator;
	copyIterator = mallocLocal(sizeof(stSortedSet_Iterator));
	avl_t_copy(copyIterator, iterator);
	return copyIterator;
}

void *st_sortedSet_getPrevious(stSortedSet_Iterator *iterator) {
	return avl_t_prev(iterator);
}
