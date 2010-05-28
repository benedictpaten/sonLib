/*
 * sonLibstList.h
 *
 *  Created on: 24 May 2010
 *      Author: benedictpaten
 */

#ifndef SONLIBLIST_H_
#define SONLIBLIST_H_

#include "sonLibGlobals.h"

/*
 * Construct a stList with zero length.
 * The destructor will not clean up the elements in the stList.
 */
st_List *st_list_construct();

/*
 * Construct a stList with size length.
 * The destructor will not clean up the elements in the stList.
 */
st_List *st_list_construct2(int32_t size);

/*
 * Construct a stList with size length.
 * The destructor will call the given destructElement function
 * for each non-null entry in the stList.
 */
st_List *st_list_construct3(int32_t size, void (*destructElement)(void *));

/*
 * Destructs the stList and, if a destructElement function was given to the constructor,
 * calls the destruct element function for each non-null element in the stList.
 */
void st_list_destruct(st_List *list);

/*
 * Returns the number of elements in the stList.
 */
int32_t st_list_length(st_List *list);

/*
 * Gets item 0 <= index < stList_length(list) from the stList.
 */
void *st_list_get(st_List *list, int32_t index);

/*
 * Sets the item at that position in the stList.
 */
void st_list_set(st_List *list, int32_t index, void *item);

/*
 * Adds the item to the end of the st_list, resizing if needed.
 */
void st_list_append(st_List *list, void *item);

/*
 * Adds all the elements in the second st_list to the end of the first, in order.
 */
void st_list_appendAll(st_List *stListToAddTo, st_List *stListToAdd);

/*
 * Returns the last element in the stList. Error if stList is empty.
 */
void *st_list_peek(st_List *list);

/*
 * Removes the last element in the stList and returns it.
 * Error if the stList is empty.
 */
void *st_list_pop(st_List *list);

/*
 * Removes and returns the item at the given index, returning the given item.
 */
void *st_list_remove(st_List *list, int32_t index);

/*
 * Removes any the first instance of this item from the given stList.
 */
void st_list_removeItem(st_List *list, void *item);

/*
 * Removes the first element in the stList and returns it. Creates an error if empty.
 */
void *st_list_removeFirst(st_List *list);

/*
 * Returns non-zero iff the stList contain one or more copies of references to the given item.
 */
int32_t st_list_contains(st_List *list, void *item);

/*
 * Copies the stList. Sets the given destruct item function to the new stList.. can
 * be null if you want no destruction of the items in that stList.
 */
st_List *st_list_copy(st_List *list, void (*destructItem)(void *));

/*
 * Reverses the stList in place.
 */
void st_list_reverse(st_List *list);

/*
 * Gets an iterator for the stList.
 */
st_ListIterator *st_list_getIterator(st_List *list);

/*
 * Destruct the stList iterator.
 */
void st_list_destructIterator(st_ListIterator *iterator);

/*
 * Gets the next item from the iterator.
 */
void *st_list_getNext(st_ListIterator *iterator);

/*
 * Gets the previous item from the iterator.
 */
void *st_list_getPrevious(st_ListIterator *iterator);

/*
 * Copies the iterator.
 */
st_ListIterator *st_list_copyIterator(st_ListIterator *iterator);

/*
 * Sorts the stList with the given cmpFn.
 */
void st_list_sort(st_List *list, int cmpFn(const void *a, const void *b));

/*
 * Gets a sorted set representation of the stList, using the given cmpFn as backing. The sorted set
 * has no defined destruct element function, so when the sorted set is destructed the elements in it and
 * in this list will not be destructed.
 */
st_SortedSet *st_list_getSortedSet(st_List *list, int (*cmpFn)(const void *a, const void *b));

#endif /* SONLIBLIST_H_ */
