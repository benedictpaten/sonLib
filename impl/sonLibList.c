/*
 * sonLibList.c
 *
 *  Created on: 24 May 2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsInternal.h"

#define MINIMUM_ARRAY_EXPAND_SIZE 5 //The minimum amount to expand the array backing a list by when it is rescaled.

/*
 * The actual datastructures backing the list
 */

struct _stList {
    void **list;
    int32_t length;
    int32_t maxLength;
    void (*destructElement)(void *);
};

struct _stListIterator {
    stList *list;
    int32_t index;
};

/*
 * The functions..
 */

stList *stList_construct() {
    return stList_construct3(0, NULL);
}

stList *stList_construct2(int32_t size) {
    return stList_construct3(size, NULL);
}

stList *stList_construct3(int32_t length, void (*destructElement)(void *)) {
    assert(length >= 0);
    stList *list = st_malloc(sizeof(stList));
    list->length = length;
    list->maxLength = length;
    list->list = st_calloc(length, sizeof(void *));
    list->destructElement = destructElement;
    return list;
}

/* free elements in list */
static void destructElements(stList *list) {
    for(int32_t i=0; i<stList_length(list); i++) { //only free up to known area of list
        if(stList_get(list, i) != NULL) {
            list->destructElement(stList_get(list, i));
        }
    }
}

void stList_destruct(stList *list) {
    if (list != NULL) {
        if (list->destructElement != NULL) {
            destructElements(list);
        }
        if(list->list != NULL) {
            free(list->list);
        }
        free(list);
    }
}

int32_t stList_length(stList *list) {
    if (list == NULL) {
        return 0;
    } else {
        return list->length;
    }
}

void *stList_get(stList *list, int32_t index) {
    assert(index >= 0);
    assert(index < stList_length(list));
    return list->list[index];
}

void stList_set(stList *list, int32_t index, void *item) {
    assert(index >= 0);
    assert(index < stList_length(list));
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

void stList_append(stList *list, void *item) {
    if(stList_length(list) >= list->maxLength) {
        list->list = st_list_appendP(list->list, &list->maxLength, list->maxLength*2 + MINIMUM_ARRAY_EXPAND_SIZE, sizeof(void *));
    }
    list->list[list->length++] = item;
}

void stList_appendAll(stList *stListToAddTo, stList *stListToAdd) {
    int32_t i;
    assert(stListToAdd != stListToAddTo);
    for(i=0; i<stList_length(stListToAdd); i++) {
        stList_append(stListToAddTo, stList_get(stListToAdd, i));
    }
}

void *stList_peek(stList *list) {
    assert(stList_length(list) > 0);
    return stList_get(list, stList_length(list)-1);
}

void *stList_pop(stList *list) {
    return stList_remove(list, stList_length(list)-1);
}

void *stList_remove(stList *list, int32_t index) {
    assert(index >= 0);
    assert(index < stList_length(list));
    void *o = stList_get(list, index);
    int32_t i;
    for(i=index+1; i<stList_length(list); i++) {
        stList_set(list, i-1, stList_get(list, i));
    }
    list->length--;
    return o;
}

void stList_removeItem(stList *list, void *item)  {
    int32_t i;
    for(i=0; i<stList_length(list); i++) {
        if(stList_get(list, i) == item) {
            stList_remove(list, i);
            return;
        }
    }
}

void *stList_removeFirst(stList *list) {
    return stList_remove(list, 0);
}

int32_t stList_contains(stList *list, void *item) {
    int32_t i;
    for(i=0; i<stList_length(list); i++) {
        if(stList_get(list, i) == item) {
            return 1;
        }
    }
    return 0;
}

stList *stList_copy(stList *list, void (*destructItem)(void *)) {
    stList *list2 = stList_construct3(0, destructItem);
    stList_appendAll(list2, list);
    return list2;
}

void stList_reverse(stList *list) {
    int32_t i, j = stList_length(list);
    for(i=0; i<j/2; i++) {
        void *o = stList_get(list, j - 1 - i);
        stList_set(list, j - 1 - i, stList_get(list, i));
        stList_set(list, i, o);
    }
}

stListIterator *stList_getIterator(stList *list) {
    stListIterator *it = st_malloc(sizeof(stListIterator));
    it->list = list;
    it->index = 0;
    return it;
}

void stList_destructIterator(stListIterator *iterator) {
    free(iterator);
}

void *stList_getNext(stListIterator *iterator) {
    if ((iterator->list == NULL) || (iterator->index >= stList_length(iterator->list))) {
        return NULL;
    } else {
        return stList_get(iterator->list, iterator->index++);
    }
}

void *stList_getPrevious(stListIterator *iterator) {
    if ((iterator->list == NULL) || (iterator->index == 0)) {
        return NULL;
    } else {
        return stList_get(iterator->list, --iterator->index);
    }
}

stListIterator *stList_copyIterator(stListIterator *iterator) {
    stListIterator *it = stList_getIterator(iterator->list);
    it->index = iterator->index;
    return it;
}

static int (*st_list_sort_cmpFn)(const void *a, const void *b);
static int st_list_sortP(const void *a, const void *b) {
    return st_list_sort_cmpFn(*((char **)a), *((char **)b));
}

void stList_sort(stList *list, int cmpFn(const void *a, const void *b)) {
    st_list_sort_cmpFn = cmpFn;
    qsort(list->list, stList_length(list), sizeof(void *), st_list_sortP);
}

stSortedSet *stList_getSortedSet(stList *list, int (*cmpFn)(const void *a, const void *b)) {
    stSortedSet *sortedSet = stSortedSet_construct3(cmpFn, NULL);
    int32_t i;
    for(i=0; i<stList_length(list); i++) {
        stSortedSet_insert(sortedSet, stList_get(list, i));
    }
    return sortedSet;
}
