/*
 * Copyright (C) 2012 by Dent Earl dentearl (a) gmail com
 *
 * Released under the MIT license, see LICENSE.txt
 */

#ifndef SONLIB_SET_H_
#define SONLIB_SET_H_

/*
 * sonLibSet.h
 *
 *  Created on: 12 July 2012
 *      Author: dentearl
 */

#include "sonLibTypes.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
 * Function which generates hash key from pointer, should work well regardless of pointer size.
 */
uint32_t stSet_pointer(const void *k);

/*
 * Constructs set, with no destructors for keys or values.
 */
stSet *stSet_construct(void);

/*
 * Constructs a set with given destructor, if null then destructor is ignored
 */
stSet *stSet_construct2(void (*destructKeys)(void *));

/*
 * Constructs a set using the given comparison functions.
 */
stSet *stSet_construct3(uint32_t (*hashKey)(const void *), int (*hashEqualsKey)(const void *, const void *),
        void (*destructKeys)(void *));

/*
 * Destructs a set.
 */
void stSet_destruct(stSet *set);

/*
 * Insert element, overiding if already present.
 */
void stSet_insert(stSet *set, void *key);

/*
 * Search for value, returns null if not present.
 */
void *stSet_search(stSet *set, void *key);

/*
 * Removes element, returning removed element.
 */
void *stSet_remove(stSet *set, void *key);

/*
 * Removes element, returning removed element and freeing key (using supplied function).
 */
void *stSet_removeAndFreeKey(stSet *set, void *key);

/*
 * Returns the number of keys in the set.
 */
int32_t stSet_size(stSet *set);

/*
 * Returns an iterator of the keys in the set.
 */
stSetIterator *stSet_getIterator(stSet *set);

/*
 * Gets the next key from the iterator.
 */
void *stSet_getNext(stSetIterator *iterator);

/*
 * Duplicates the iterator.
 */
stSetIterator *stSet_copyIterator(stSetIterator *iterator);

/*
 * Destructs the iterator.
 */
void stSet_destructIterator(stSetIterator *iterator);

/*
 * Gets the keys in the set as list.
 */
stList *stSet_getKeys(stSet *set);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // SONLIB_SET_H_
