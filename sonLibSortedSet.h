#ifndef SON_LIB_SORTED_SET_H_
#define SON_LIB_SORTED_SET_H_

#include "sonLibGlobals.h"

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//Sorted set functions
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

/*
 * Constructs a sorted set, using the given comparison function.
 */
stSortedSet *st_sortedSet_construct(int32_t (*compareFn)(const void *, const void *, void *));

/*
 * Destructs the sorted set, applying the destruct function to each element.
 */
void st_sortedSet_destruct(stSortedSet *sortedSet, void (*destructElementFn)(void *, void *));

/*
 * Inserts the object into the sorted set.
 */
void st_sortedSet_insert(stSortedSet *sortedSet, void *object);

/*
 * Finds the objects in the sorted set, or returns null.
 */
void *st_sortedSet_find(stSortedSet *sortedSet, void *object);

/*
 * Deletes the object in the sorted set.
 */
void st_sortedSet_delete(stSortedSet *sortedSet, void *object);

/*
 * Gets the number of elements in the sorted set.
 */
int32_t st_sortedSet_getLength(stSortedSet *sortedSet);

/*
 * Gets the first element (with lowest value), in the sorted set.
 */
void *st_sortedSet_getFirst(stSortedSet *items);

/*
 * Gets the last element in the sorted set.
 */
void *st_sortedSet_getLast(stSortedSet *items);

/*
 * Constructs an iterator for the sorted set.
 */
stSortedSet_Iterator *st_sortedSet_getIterator(stSortedSet *items);

/*
 * Destructs an iterator for the sorted set.
 */
void st_sortedSet_destructIterator(stSortedSet_Iterator *iterator);

/*
 * Gets next element in the sorted set.
 */
void *st_sortedSet_getNext(stSortedSet_Iterator *iterator);

/*
 * Gets the previous element in the sorted set.
 */
void *st_sortedSet_getPrevious(stSortedSet_Iterator *iterator);

/*
 * Copies the iterator.
 */
stSortedSet_Iterator *st_sortedSet_copyIterator(stSortedSet_Iterator *iterator);

#endif
