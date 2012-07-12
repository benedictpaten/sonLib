/*
 * Copyright (C) 2006-2012 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibHash.c
 *
 *  Created on: 4 Apr 2010
 *      Author: benedictpaten
 */
#include "sonLibGlobalsInternal.h"
#include "hashTableC.h"
#include "hashTableC_itr.h"

struct _stHash {
    struct hashtable *hash;
    bool destructKeys, destructValues;
};

uint32_t stHash_pointer(const void *k) {
    /*if (sizeof(const void *) > 4) {
        int64_t key = (int64_t) (size_t) k;
        key = (~key) + (key << 18); // key = (key << 18) - key - 1;
        key = key ^ (key >> 31);
        key = key * 21; // key = (key + (key << 2)) + (key << 4);
        key = key ^ (key >> 11);
        key = key + (key << 6);
        key = key ^ (key >> 22);
        return (uint32_t) key;
    }*/
    return (uint32_t) (size_t) k; //Just use the low order bits
}

static int stHash_equalKey(const void *key1, const void *key2) {
    return key1 == key2;
}

stHash *stHash_construct(void) {
    return stHash_construct3(stHash_pointer, stHash_equalKey, NULL, NULL);
}

stHash *stHash_construct2(void(*destructKeys)(void *), void(*destructValues)(void *)) {
    return stHash_construct3(stHash_pointer, stHash_equalKey, destructKeys, destructValues);
}

stHash *stHash_construct3(uint32_t(*hashKey)(const void *), int(*hashEqualsKey)(const void *, const void *), void(*destructKeys)(void *),
        void(*destructValues)(void *)) {
    stHash *hash = st_malloc(sizeof(stHash));
    hash->hash = create_hashtable(0, hashKey, hashEqualsKey, destructKeys, destructValues);
    hash->destructKeys = destructKeys != NULL;
    hash->destructValues = destructValues != NULL;
    return hash;
}

void stHash_destruct(stHash *hash) {
    hashtable_destroy(hash->hash, hash->destructValues, hash->destructKeys);
    free(hash);
}

void stHash_insert(stHash *hash, void *key, void *value) {
    if (stHash_search(hash, key) != NULL) { //This will ensure we don't end up with duplicate keys..
        stHash_remove(hash, key);
    }
    hashtable_insert(hash->hash, key, value);
}

void *stHash_search(stHash *hash, void *key) {
    return hashtable_search(hash->hash, key);
}

void *stHash_remove(stHash *hash, void *key) {
    return hashtable_remove(hash->hash, key, 0);
}

void *stHash_removeAndFreeKey(stHash *hash, void *key) {
    return hashtable_remove(hash->hash, key, 1);
}

int32_t stHash_size(stHash *hash) {
    return hashtable_count(hash->hash);
}

stHashIterator *stHash_getIterator(stHash *hash) {
    return hashtable_iterator(hash->hash);
}

void *stHash_getNext(stHashIterator *iterator) {
    if (iterator->e != NULL) {
        void *o = hashtable_iterator_key(iterator);
        hashtable_iterator_advance(iterator);
        return o;
    }
    return NULL;
}

stHashIterator *stHash_copyIterator(stHashIterator *iterator) {
    stHashIterator *iterator2 = st_malloc(sizeof(stHashIterator));
    iterator2->h = iterator->h;
    iterator2->e = iterator->e;
    iterator2->parent = iterator->parent;
    iterator2->index = iterator->index;
    return iterator2;
}

void stHash_destructIterator(stHashIterator *iterator) {
    free(iterator);
}

stList *stHash_getKeys(stHash *hash) {
    stList *list = stList_construct();
    stHashIterator *iterator = stHash_getIterator(hash);
    void *item;
    while ((item = stHash_getNext(iterator)) != NULL) {
        stList_append(list, item);
    }
    stHash_destructIterator(iterator);
    return list;
}

stList *stHash_getValues(stHash *hash) {
    stList *list = stList_construct();
    stHashIterator *iterator = stHash_getIterator(hash);
    void *item;
    while ((item = stHash_getNext(iterator)) != NULL) {
        stList_append(list, stHash_search(hash, item));
    }
    stHash_destructIterator(iterator);
    return list;
}

/*
 * Useful hash keys/equals functions
 */

uint32_t stHash_stringKey(const void *k) {
    // djb2
    // This algorithm was first reported by Dan Bernstein
    // many years ago in comp.lang.c
    //
    uint32_t hash = 0; //5381;
    int c;
    char *cA;
    cA = (char *) k;
    while ((c = *cA++) != '\0') {
        hash = c + (hash << 6) + (hash << 16) - hash;
        //hash = ((hash << 5) + hash) + c; // hash*33 + c
    }
    return hash;
}

int stHash_stringEqualKey(const void *key1, const void *key2) {
    return strcmp(key1, key2) == 0;
}

stHash *stHash_invert(stHash *hash, uint32_t(*hashKey)(const void *), int(*equalsFn)(const void *, const void *),
        void(*destructKeys)(void *), void(*destructValues)(void *)) {
    /*
     * Inverts the hash.
     */
    stHash *invertedHash = stHash_construct3(hashKey, equalsFn, destructKeys, destructValues);
    stHashIterator *hashIt = stHash_getIterator(hash);
    void *key;
    while ((key = stHash_getNext(hashIt)) != NULL) {
        void *value = stHash_search(hash, key);
        assert(value != NULL);
        if (stHash_search(invertedHash, value) == NULL) {
            stHash_insert(invertedHash, value, key);
        }
    }
    stHash_destructIterator(hashIt);
    return invertedHash;
}
