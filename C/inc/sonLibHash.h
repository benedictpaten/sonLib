#ifndef SONLIB_HASH_H_
#define SONLIB_HASH_H_

/*
 * sonLibHash.h
 *
 *  Created on: 4 Apr 2010
 *      Author: benedictpaten
 */

#include "sonLibTypes.h"

/*
 * Constructs hash, with no destructors for keys or values.
 */
stHash *stHash_construct(void);

/*
 * Constructs a hash with given destructors, if null then destructors are ignored
 */
stHash *stHash_construct2(void (*destructKeys)(void *), void (*destructValues)(void *));

/*
 * Constructs a hash using the given comparison functions.
 */
stHash *stHash_construct3(uint32_t (*hashKey)(const void *), int (*hashEqualsKey)(const void *, const void *),
        void (*destructKeys)(void *), void (*destructValues)(void *));

/*
 * Destructs a hash.
 */
void stHash_destruct(stHash *hash);

/*
 * Insert element, overiding if already present.
 */
void stHash_insert(stHash *hash, void *key, void *value);

/*
 * Search for value, returns null if not present.
 */
void *stHash_search(stHash *hash, void *key);

/*
 * Removes element, returning removed element.
 */
void *stHash_remove(stHash *hash, void *key);

/*
 * Returns the number of key/value pairs in the hash.
 */
int32_t stHash_size(stHash *hash);

/*
 * Returns an iterator of the keys in the hash.
 */
stHashIterator *stHash_getIterator(stHash *hash);

/*
 * Gets the next key from the iterator.
 */
void *stHash_getNext(stHashIterator *iterator);

/*
 * Duplicates the iterator.
 */
stHashIterator *stHash_copyIterator(stHashIterator *iterator);

/*
 * Destructs the iterator.
 */
void stHash_destructIterator(stHashIterator *iterator);

/*
 * Gets the keys in the hash as list.
 */
stList *stHash_getKeys(stHash *hash);

/*
 * Gets the values in the hash as a list.
 */
stList *stHash_getValues(stHash *hash);

/*
 * Useful hash keys..
 */

/*
 * A hash function for a char string.
 */
uint32_t stHash_stringKey( const void *k );

/*
 * A hash equals function for two char strings.
 */
int stHash_stringEqualKey( const void *key1, const  void *key2 );

#endif
