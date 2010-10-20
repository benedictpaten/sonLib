/*
 * sonLibstList.h
 *
 *  Created on: 24 May 2010
 *      Author: benedictpaten
 */

#ifndef SONLIB_LIST_H_
#define SONLIB_LIST_H_

#include "sonLibTypes.h"

/*
 * Construct a stList with zero length.
 * The destructor will not clean up the elements in the stList.
 */
stList *stList_construct(void);

/*
 * Construct a stList with size length.
 * The destructor will not clean up the elements in the stList.
 */
stList *stList_construct2(int32_t size);

/*
 * Construct a stList with size length.
 * The destructor will call the given destructElement function
 * for each non-null entry in the stList.
 */
stList *stList_construct3(int32_t size, void (*destructElement)(void *));

/*
 * Destructs the stList and, if a destructElement function was given to the constructor,
 * calls the destruct element function for each non-null element in the stList.
 * The list maybe NULL.
 */
void stList_destruct(stList *list);

/*
 * Returns the number of elements in the stList.  The list maybe NULL,
 * in which case zero is returned.  This allows for NULL to be used
 * as an efficient way of returning an empty list.
 */
int32_t stList_length(stList *list);

/*
 * Gets item 0 <= index < stList_length(list) from the stList.
 */
void *stList_get(stList *list, int32_t index);

/*
 * Sets the item at that position in the stList.
 */
void stList_set(stList *list, int32_t index, void *item);

/*
 * Adds the item to the end of the st_list, resizing if needed.
 */
void stList_append(stList *list, void *item);

/*
 * Adds all the elements in the second st_list to the end of the first, in order.
 */
void stList_appendAll(stList *stListToAddTo, stList *stListToAdd);

/*
 * Returns the last element in the stList. Error if stList is empty.
 */
void *stList_peek(stList *list);

/*
 * Removes the last element in the stList and returns it.
 * Error if the stList is empty.
 */
void *stList_pop(stList *list);

/*
 * Removes and returns the item at the given index, returning the given item.
 */
void *stList_remove(stList *list, int32_t index);

/*
 * Removes any the first instance of this item from the given stList.
 */
void stList_removeItem(stList *list, void *item);

/*
 * Removes the first element in the stList and returns it. Creates an error if empty.
 */
void *stList_removeFirst(stList *list);

/*
 * Returns non-zero iff the stList contain one or more copies of references to the given item.
 */
int32_t stList_contains(stList *list, void *item);

/*
 * Copies the stList. Sets the given destruct item function to the new stList.. can
 * be null if you want no destruction of the items in that stList.
 */
stList *stList_copy(stList *list, void (*destructItem)(void *));

/*
 * Reverses the stList in place.
 */
void stList_reverse(stList *list);

/*
 * Gets an iterator for the stList.  The list maybe NULL, in which case a
 * iterator that only returns NULL is created..  This allows for NULL to be
 * used as an efficient way of returning an empty list.
 */
stListIterator *stList_getIterator(stList *list);

/*
 * Destruct the stList iterator.
 */
void stList_destructIterator(stListIterator *iterator);

/*
 * Gets the next item from the iterator.
 */
void *stList_getNext(stListIterator *iterator);

/*
 * Gets the previous item from the iterator.
 */
void *stList_getPrevious(stListIterator *iterator);

/*
 * Copies the iterator.
 */
stListIterator *stList_copyIterator(stListIterator *iterator);

/*
 * Sorts the stList with the given cmpFn.
 */
void stList_sort(stList *list, int cmpFn(const void *a, const void *b));

/*
 * Gets a sorted set representation of the stList, using the given cmpFn as backing. The sorted set
 * has no defined destruct element function, so when the sorted set is destructed the elements in it and
 * in this list will not be destructed. If the cmpFn is NULL then we use the default cmpFn.
 */
stSortedSet *stList_getSortedSet(stList *list, int (*cmpFn)(const void *a, const void *b));

/*
 * Sets the destructor of the list.
 */
void stList_setDestructor(stList *list, void (*destructElement)(void *));

#endif /* SONLIBLIST_H_ */
