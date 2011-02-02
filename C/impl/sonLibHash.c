/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
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

static uint32_t stHash_key( const void *k ) {
    return (uint32_t)(size_t)k;
}

static int stHash_equalKey( const void *key1, const void *key2 ) {
    return key1 == key2;
}

stHash *stHash_construct(void) {
    return stHash_construct3(stHash_key, stHash_equalKey, NULL, NULL);
}

stHash *stHash_construct2(void (*destructKeys)(void *), void (*destructValues)(void *)) {
    return stHash_construct3(stHash_key, stHash_equalKey, destructKeys, destructValues);
}

stHash *stHash_construct3(uint32_t (*hashKey)(const void *), int (*hashEqualsKey)(const void *, const void *),
        void (*destructKeys)(void *), void (*destructValues)(void *)) {
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
    if(stHash_search(hash, key) != NULL) { //This will ensure we don't end up with duplicate keys..
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

int32_t stHash_size(stHash *hash) {
    return hashtable_count(hash->hash);
}

stHashIterator *stHash_getIterator(stHash *hash) {
    return hashtable_iterator(hash->hash);
}

void *stHash_getNext(stHashIterator *iterator) {
    if(iterator->e != NULL) {
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
    while((item = stHash_getNext(iterator)) != NULL) {
        stList_append(list, item);
    }
    stHash_destructIterator(iterator);
    return list;
}

stList *stHash_getValues(stHash *hash) {
    stList *list = stList_construct();
    stHashIterator *iterator = stHash_getIterator(hash);
    void *item;
    while((item = stHash_getNext(iterator)) != NULL) {
        stList_append(list, stHash_search(hash, item));
    }
    stHash_destructIterator(iterator);
    return list;
}

/*
 * Useful hash keys/equals functions
 */

uint32_t stHash_stringKey( const void *k ) {
    uint32_t i, j;
    const char *cA;

    cA = k;
    j = 0;
    for(i=0; i<strlen(cA) && i<10; i++) {
        j += cA[i];
    }
    return j;
    }

int stHash_stringEqualKey( const void *key1, const  void *key2 ) {
    return strcmp(key1, key2) == 0;
}
