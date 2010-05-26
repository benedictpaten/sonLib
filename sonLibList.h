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
stList *st_list_construct();

/*
 * Construct a stList with size length.
 * The destructor will not clean up the elements in the stList.
 */
stList *st_list_construct2(int32_t size);

/*
 * Construct a stList with size length.
 * The destructor will call the given destructElement function
 * for each non-null entry in the stList.
 */
stList *st_list_construct3(int32_t size, void (*destructElement)(void *));

/*
 * Destructs the stList and, if a destructElement function was given to the constructor,
 * calls the destruct element function for each non-null element in the stList.
 */
void st_list_destruct(stList *list);

/*
 * Returns the number of elements in the stList.
 */
int32_t st_list_length(stList *list);

/*
 * Gets item 0 <= index < stList_length(list) from the stList.
 */
void *st_list_get(stList *list, int32_t index);

/*
 * Sets the item at that position in the stList.
 */
void st_list_set(stList *list, int32_t index, void *item);

/*
 * Adds the item to the end of the st_list, resizing if needed.
 */
void st_list_append(stList *list, void *item);

/*
 * Adds all the elements in the second st_list to the end of the first, in order.
 */
void st_list_appendAll(stList *stListToAddTo, stList *stListToAdd);

/*
 * Returns the last element in the stList. Error if stList is empty.
 */
void *st_list_peek(stList *list);

/*
 * Removes the last element in the stList and returns it.
 * Error if the stList is empty.
 */
void *st_list_pop(stList *list);

/*
 * Removes and returns the item at the given index, returning the given item.
 */
void *st_list_remove(stList *list, int32_t index);

/*
 * Removes any the first instance of this item from the given stList.
 */
void st_list_removeItem(stList *list, void *item);

/*
 * Removes the first element in the stList and returns it. Creates an error if empty.
 */
void *st_list_removeFirst(stList *list);

/*
 * Returns non-zero iff the stList contain one or more copies of references to the given item.
 */
int32_t st_list_contains(stList *list, void *item);

/*
 * Copies the stList. Sets the given destruct item function to the new stList.. can
 * be null if you want no destruction of the items in that stList.
 */
stList *st_list_copy(stList *list, void (*destructItem)(void *));

/*
 * Reverses the stList in place.
 */
void st_list_reverse(stList *list);

/*
 * Gets an iterator for the stList.
 */
stListIterator *st_list_getIterator(stList *list);

/*
 * Destruct the stList iterator.
 */
void st_list_destructIterator(stListIterator *iterator);

/*
 * Gets the next item from the iterator.
 */
void *st_list_getNext(stListIterator *iterator);

/*
 * Gets the previous item from the iterator.
 */
void *st_list_getPrevious(stListIterator *iterator);

/*
 * Copies the iterator.
 */
stListIterator *st_list_copyIterator(stListIterator *iterator);

/*
 * Sorts the stList with the given cmpFn.
 */
void st_list_sort(stList *list, int cmpFn(const void *a, const void *b));

/*
 * Gets a sorted set representation of the stList, using the given cmpFn as backing.
 */
stSortedSet *st_list_getSortedSet(stList *list, int (*cmpFn)(const void *a, const void *b));

#endif /* SONLIBLIST_H_ */
