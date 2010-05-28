#ifndef SONLIB_SORTED_SET_H_
#define SONLIB_SORTED_SET_H_

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
stSortedSet *stSortedSet_construct();

/*
 * Constructs a sorted set, using pointer based comparison function,
 * and the given destruct element function, which will be run on each element when
 * the set is destructed.
 */
stSortedSet *stSortedSet_construct2(void (*destructElementFn)(void *));

/*
 * Constructs a sorted set, using the given comparison function and destruct element function.
 * If destruct element function is null then it is ignored.
 */
stSortedSet *stSortedSet_construct3(int (*compareFn)(const void *, const void *),
									  void (*destructElementFn)(void *));

/*
 * Destructs the sorted set.
 */
void stSortedSet_destruct(stSortedSet *sortedSet);

/*
 * Inserts the object into the sorted set.
 */
void stSortedSet_insert(stSortedSet *sortedSet, void *object);

/*
 * Finds the objects in the sorted set, or returns null.
 */
void *stSortedSet_search(stSortedSet *sortedSet, void *object);

/*
 * Deletes the object in the sorted set.
 */
void stSortedSet_remove(stSortedSet *sortedSet, void *object);

/*
 * Gets the number of elements in the sorted set.
 */
int32_t stSortedSet_size(stSortedSet *sortedSet);

/*
 * Gets the first element (with lowest value), in the sorted set.
 */
void *stSortedSet_getFirst(stSortedSet *items);

/*
 * Gets the last element in the sorted set.
 */
void *stSortedSet_getLast(stSortedSet *items);

/*
 * Constructs an iterator for the sorted set.
 */
stSortedSetIterator *stSortedSet_getIterator(stSortedSet *items);

/*
 * Destructs an iterator for the sorted set.
 */
void stSortedSet_destructIterator(stSortedSetIterator *iterator);

/*
 * Gets next element in the sorted set.
 */
void *stSortedSet_getNext(stSortedSetIterator *iterator);

/*
 * Gets the previous element in the sorted set.
 */
void *stSortedSet_getPrevious(stSortedSetIterator *iterator);

/*
 * Copies the iterator.
 */
stSortedSetIterator *stSortedSet_copyIterator(stSortedSetIterator *iterator);

/*
 * Gets a stList version of the sorted set, sorted in the order of the sorted set.
 * No destructor is defined for the list, so destroying the list will not destroy
 * the elements in it or the sorted set.
 */
stList *stSortedSet_getList(stSortedSet *sortedSet);

#endif
