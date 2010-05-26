#ifndef SON_LIB_HASH_H_
#define SON_LIB_HASH_H_

/*
 * sonLibHash.h
 *
 *  Created on: 4 Apr 2010
 *      Author: benedictpaten
 */

#include "sonLibGlobals.h"

/*
 * Constructs hash, with no destructors for keys or values.
 */
st_Hash *st_hash_construct();

/*
 * Constructs a hash with given destructors, if null then destructors are ignored
 */
st_Hash *st_hash_construct2(void (*destructKeys)(void *), void (*destructValues)(void *));

/*
 * Constructs a hash using the given comparison functions.
 */
st_Hash *st_hash_construct3(uint32_t (*hashKey)(void *), int32_t (*hashEqualsKey)(void *, void *),
		void (*destructKeys)(void *), void (*destructValues)(void *));

/*
 * Destructs a hash.
 */
void st_hash_destruct(st_Hash *hash);

/*
 * Insert element, overiding if already present.
 */
void st_hash_insert(st_Hash *hash, void *key, void *value);

/*
 * Search for value, returns null if not present.
 */
void *st_hash_search(st_Hash *hash, void *key);

/*
 * Removes element, returning removed element.
 */
void *st_hash_remove(st_Hash *hash, void *key);

/*
 * Returns the number of key/value pairs in the hash.
 */
int32_t st_hash_size(st_Hash *hash);

/*
 * Returns an iterator of the keys in the hash.
 */
st_HashIterator *st_hash_getIterator(st_Hash *hash);

/*
 * Gets the next key from the iterator.
 */
void *st_hash_getNext(st_HashIterator *iterator);

/*
 * Duplicates the iterator.
 */
st_HashIterator *st_hash_copyIterator(st_HashIterator *iterator);

/*
 * Destructs the iterator.
 */
void st_hash_destructIterator(st_HashIterator *iterator);

#endif
