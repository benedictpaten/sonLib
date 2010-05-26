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
st_SortedSet *st_sortedSet_construct(int32_t (*compareFn)(const void *, const void *, void *));

/*
 * Destructs the sorted set, applying the destruct function to each element.
 */
void st_sortedSet_destruct(st_SortedSet *sortedSet, void (*destructElementFn)(void *, void *));

/*
 * Inserts the object into the sorted set.
 */
void st_sortedSet_insert(st_SortedSet *sortedSet, void *object);

/*
 * Finds the objects in the sorted set, or returns null.
 */
void *st_sortedSet_find(st_SortedSet *sortedSet, void *object);

/*
 * Deletes the object in the sorted set.
 */
void st_sortedSet_delete(st_SortedSet *sortedSet, void *object);

/*
 * Gets the number of elements in the sorted set.
 */
int32_t st_sortedSet_getLength(st_SortedSet *sortedSet);

/*
 * Gets the first element (with lowest value), in the sorted set.
 */
void *st_sortedSet_getFirst(st_SortedSet *items);

/*
 * Gets the last element in the sorted set.
 */
void *st_sortedSet_getLast(st_SortedSet *items);

/*
 * Constructs an iterator for the sorted set.
 */
st_SortedSetIterator *st_sortedSet_getIterator(st_SortedSet *items);

/*
 * Destructs an iterator for the sorted set.
 */
void st_sortedSet_destructIterator(st_SortedSetIterator *iterator);

/*
 * Gets next element in the sorted set.
 */
void *st_sortedSet_getNext(st_SortedSetIterator *iterator);

/*
 * Gets the previous element in the sorted set.
 */
void *st_sortedSet_getPrevious(st_SortedSetIterator *iterator);

/*
 * Copies the iterator.
 */
st_SortedSetIterator *st_sortedSet_copyIterator(st_SortedSetIterator *iterator);

#endif
