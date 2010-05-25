/*
 * sonLibList.h
 *
 *  Created on: 24 May 2010
 *      Author: benedictpaten
 */

#ifndef SONLIBLIST_H_
#define SONLIBLIST_H_

#include "sonLibGlobals.h"

/*
 * Construct a list with zero length.
 * The destructor will not clean up the elements in the list.
 */
List *list_construct();

/*
 * Construct a list with size length.
 * The destructor will not clean up the elements in the list.
 */
List *list_construct2(int32_t size);

/*
 * Construct a list with size length.
 * The destructor will call the given destructElement function
 * for each non-null entry in the list.
 */
List *list_construct3(int32_t size, void (*destructElement)(void *));

/*
 * Destructs the list and, if a destructElement function was given to the constructor,
 * calls the destruct element function for each non-null element in the list.
 */
void list_destruct(List *list);

/*
 * Adds the item to the end of the list, resizing if needed.
 */
void list_append(List *list, void *item);

/*
 * Adds all the elements in the second list to the end of the first.
 */
void list_appendAll(List *list, List *listToAdd);

/*
 * Returns the number of elements in the list.
 */
int32_t list_length(List *list);

/*
 * Gets item 0 <= index < list_length(list) from the list.
 */
void *list_get(List *list, int32_t index);

/*
 * Returns the last element in the list. Error if list is empty.
 */
void *list_peek(List *list);

/*
 * Removes the last element in the list and returns it.
 * Error if the list is empty.
 */
void *list_pop(List *list);

/*
 * Removes the first instance of this item from the given list.
 * Returns non-zero if an item is removed, zero if it is not found in the list.
 */
int32_t list_remove(List *list, void *item);

/*
 * Removes the first element in the list.
 */
void *list_removeFirst(List *list);

/*
 * Returns non-zero iff the list contain one or more copies of references to the given item.
 */
int32_t list_contains(List *list, void *item);

/*
 * Copies the list.
 */
List *list_copy(List *list);

/*
 * Reverses the list in place.
 */
void list_reverse(List *list);

/*
 * Gets an iterator for the list.
 */
ListIterator *list_getIterator(List *list);

/*
 * Destruct the list iterator.
 */
void list_destructIterator(ListIterator *iterator);

/*
 * Gets the next item from the iterator.
 */
void *list_getNext(ListIterator *iterator);

/*
 * Gets the previous item from the iterator.
 */
void *list_getPrevious(ListIterator *iterator);

/*
 * Copies the iterator.
 */
ListIterator *list_copyIterator(ListIterator *iterator);

/*
 * Gets a sorted set representation of the list.
 */
SortedSet *list_getSortedSet(List *list);

/*
 * Sorts the list with the given cmpFn.
 */
void list_sort(List *list, int32_t cmpFn(void *a, void *b));

#endif /* SONLIBLIST_H_ */
