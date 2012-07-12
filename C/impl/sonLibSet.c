/*
 * Copyright (C) 20 by Dent Earl dentearl (a) gmail com
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibSet.c
 *
 *  Created on: 12 July 2012
 *      Author: dentearl
 */
#include "sonLibGlobalsInternal.h"
#include "sonLibHash.h"
#include "sonLibSet.h"

struct _stSet {
    stHash *hash;
};
struct _stSetIterator {
    stHashIterator *hashIterator;
};

uint32_t stSet_pointer(const void *k) {
    return (uint32_t) (size_t) k; // Just use the low order bits
}
static int stSet_equalKey(const void *key1, const void *key2) {
    return key1 == key2;
}
stSet *stSet_construct(void) {
    return stSet_construct3(stSet_pointer, stSet_equalKey, NULL);
}
stSet *stSet_construct2(void(*destructKeys)(void *)) {
    return stSet_construct3(stSet_pointer, stSet_equalKey, destructKeys);
}
stSet *stSet_construct3(uint32_t(*hashKey)(const void *), int(*hashEqualsKey)(const void *, const void *), void(*destructKeys)(void *)) {
    stSet *set = st_malloc(sizeof(*set));
    set->hash = stHash_construct3(hashKey, hashEqualsKey, destructKeys, NULL);
    return set;
}
void stSet_destruct(stSet *set) {
    stHash_destruct(set->hash);
    free(set);
}
void stSet_insert(stSet *set, void *key) {
    if (stSet_search(set, key) != NULL) { // This will ensure we don't end up with duplicate keys..
        stSet_remove(set, key);
    }
    stHash_insert(set->hash, key, key);
}
void *stSet_search(stSet *set, void *key) {
    return stHash_search(set->hash, key);
}
void *stSet_remove(stSet *set, void *key) {
    return stHash_remove(set->hash, key);
}
void *stSet_removeAndFreeKey(stSet *set, void *key) {
    return stHash_removeAndFreeKey(set->hash, key);
}
int32_t stSet_size(stSet *set) {
    return stHash_size(set->hash);
}
stSetIterator *stSet_getIterator(stSet *set) {
    stSetIterator *sit = st_malloc(sizeof(*sit));
    sit->hashIterator = stHash_getIterator(set->hash);
    return sit;
}
void *stSet_getNext(stSetIterator *iterator) {
    return stHash_getNext(iterator->hashIterator);
}
stSetIterator *stSet_copyIterator(stSetIterator *iterator) {
    stSetIterator *iterator2 = st_malloc(sizeof(*iterator2));
    iterator2->hashIterator = stHash_copyIterator(iterator->hashIterator);
    return iterator2;
}
void stSet_destructIterator(stSetIterator *iterator) {
    stHash_destructIterator(iterator->hashIterator);
    free(iterator);
}
stList *stSet_getKeys(stSet *set) {
    stList *list = stList_construct();
    stSetIterator *iterator = stSet_getIterator(set);
    void *item;
    while ((item = stSet_getNext(iterator)) != NULL) {
        stList_append(list, item);
    }
    stSet_destructIterator(iterator);
    return list;
}
