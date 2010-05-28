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
 * Constructs a sorted set, using pointer based comparison function.
 */
st_SortedSet *st_sortedSet_construct();

/*
 * Constructs a sorted set, using pointer based comparison function,
 * and the given destruct element function, which will be run on each element when
 * the set is destructed.
 */
st_SortedSet *st_sortedSet_construct2(void (*destructElementFn)(void *));

/*
 * Constructs a sorted set, using the given comparison function and destruct element function.
 * If destruct element function is null then it is ignored.
 */
st_SortedSet *st_sortedSet_construct3(int (*compareFn)(const void *, const void *),
									  void (*destructElementFn)(void *));

/*
 * Destructs the sorted set.
 */
void st_sortedSet_destruct(st_SortedSet *sortedSet);

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

/*
 * Gets a stList version of the sorted set, sorted in the order of the sorted set.
 * No destructor is defined for the list, so destroying the list will not destroy
 * the elements in it or the sorted set.
 */
st_List *stSortedSet_getList(st_SortedSet *sortedSet);

#endif
