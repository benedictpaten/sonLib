/*
 * sonLibList.c
 *
 *  Created on: 24 May 2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsPrivate.h"

st_List *st_list_construct() {
	return st_list_construct3(0, NULL);
}

st_List *st_list_construct2(int32_t size) {
	return st_list_construct3(size, NULL);
}

st_List *st_list_construct3(int32_t length, void (*destructElement)(void *)) {
	assert(length >= 0);
	st_List *list = st_malloc(sizeof(st_List));
	list->length = length;
	list->maxLength = length;
    list->list = st_calloc(length, sizeof(void *));
	list->destructElement = destructElement;
	return list;
}

void st_list_destruct(st_List *list) {
	int32_t i;
	if (list->destructElement != NULL) {
		for(i=0; i<st_list_length(list); i++) { //only free up to known area of list
			if(st_list_get(list, i) != NULL) {
				list->destructElement(st_list_get(list, i));
			}
		}
	}
	if(list->list != NULL) {
		free(list->list);
	}
	free(list);
}

int32_t st_list_length(st_List *list) {
	return list->length;
}

void *st_list_get(st_List *list, int32_t index) {
	assert(index >= 0);
	assert(index < st_list_length(list));
	return list->list[index];
}

void st_list_set(st_List *list, int32_t index, void *item) {
	assert(index >= 0);
	assert(index < st_list_length(list));
	list->list[index] = item;
}

static void *st_list_appendP(void *current, int32_t *currentSize, int32_t newSize, int32_t base) {
    assert(*currentSize <= newSize);
    void *new;
    new = memcpy(st_malloc(base*newSize), current, base*(*currentSize));
    if(current != NULL) {
        free(current);
    }
    *currentSize = newSize;
    return new;
}

void st_list_append(st_List *list, void *item) {
	if(st_list_length(list) >= list->maxLength) {
		list->list = st_list_appendP(list->list, &list->maxLength, list->maxLength*2 + TINY_CHUNK_SIZE, sizeof(void *));
	}
	list->list[list->length++] = item;
}

void st_list_appendAll(st_List *stListToAddTo, st_List *stListToAdd) {
	int32_t i;
	assert(stListToAdd != stListToAddTo);
	for(i=0; i<st_list_length(stListToAdd); i++) {
		st_list_append(stListToAddTo, st_list_get(stListToAdd, i));
	}
}

void *st_list_peek(st_List *list) {
	assert(st_list_length(list) > 0);
	return st_list_get(list, st_list_length(list)-1);
}

void *st_list_pop(st_List *list) {
	return st_list_remove(list, st_list_length(list)-1);
}

void *st_list_remove(st_List *list, int32_t index) {
	assert(index >= 0);
	assert(index < st_list_length(list));
	void *o = st_list_get(list, index);
	int32_t i;
	for(i=index+1; i<st_list_length(list); i++) {
		st_list_set(list, i-1, st_list_get(list, i));
	}
	list->length--;
	return o;
}

void st_list_removeItem(st_List *list, void *item)  {
	int32_t i;
	for(i=0; i<st_list_length(list); i++) {
		if(st_list_get(list, i) == item) {
			st_list_remove(list, i);
			return;
		}
	}
}

void *st_list_removeFirst(st_List *list) {
	return st_list_remove(list, 0);
}

int32_t st_list_contains(st_List *list, void *item) {
	int32_t i;
	for(i=0; i<st_list_length(list); i++) {
		if(st_list_get(list, i) == item) {
			return 1;
		}
	}
	return 0;
}

st_List *st_list_copy(st_List *list, void (*destructItem)(void *)) {
	st_List *list2 = st_list_construct3(0, destructItem);
	st_list_appendAll(list2, list);
	return list2;
}

void st_list_reverse(st_List *list) {
	int32_t i, j = st_list_length(list);
	for(i=0; i<j/2; i++) {
		void *o = st_list_get(list, j - 1 - i);
		st_list_set(list, j - 1 - i, st_list_get(list, i));
		st_list_set(list, i, o);
	}
}

st_ListIterator *st_list_getIterator(st_List *list) {
	st_ListIterator *it = st_malloc(sizeof(st_ListIterator));
	it->list = list;
	it->index = 0;
	return it;
}

void st_list_destructIterator(st_ListIterator *iterator) {
	free(iterator);
}

void *st_list_getNext(st_ListIterator *iterator) {
	return iterator->index < st_list_length(iterator->list) ? st_list_get(iterator->list, iterator->index++) : NULL;
}

void *st_list_getPrevious(st_ListIterator *iterator) {
	return iterator->index > 0 ? st_list_get(iterator->list, --iterator->index) : NULL;
}

st_ListIterator *st_list_copyIterator(st_ListIterator *iterator) {
	st_ListIterator *it = st_list_getIterator(iterator->list);
	it->index = iterator->index;
	return it;
}

static int (*st_list_sort_cmpFn)(const void *a, const void *b);
static int st_list_sortP(const void *a, const void *b) {
	return st_list_sort_cmpFn(*((char **)a), *((char **)b));
}

void st_list_sort(st_List *list, int cmpFn(const void *a, const void *b)) {
	st_list_sort_cmpFn = cmpFn;
	qsort(list->list, st_list_length(list), sizeof(void *), st_list_sortP);
}

static int (*st_list_getSortedSet_cmpFn)(const void *a, const void *b);
static int32_t st_list_getSortedSetP(const void *a, const void *b, void *ex) {
	return st_list_getSortedSet_cmpFn(a, b);
}

st_SortedSet *st_list_getSortedSet(st_List *list, int (*cmpFn)(const void *a, const void *b)) {
	st_list_getSortedSet_cmpFn = cmpFn;
	st_SortedSet *sortedSet = st_sortedSet_construct(st_list_getSortedSetP);
	int32_t i;
	for(i=0; i<st_list_length(list); i++) {
		st_sortedSet_insert(sortedSet, st_list_get(list, i));
	}
	return sortedSet;
}
