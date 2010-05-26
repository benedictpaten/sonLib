/*
 * sonLibHash.c
 *
 *  Created on: 4 Apr 2010
 *      Author: benedictpaten
 */
#include "sonLibGlobalsPrivate.h"

stHash *st_hash_construct() {
	return st_hash_construct3(hashtable_key, hashtable_equalKey, NULL, NULL);
}

stHash *st_hash_construct2(void (*destructKeys)(void *), void (*destructValues)(void *)) {
	return st_hash_construct3(hashtable_key, hashtable_equalKey, destructKeys, destructValues);
}

stHash *st_hash_construct3(uint32_t (*hashKey)(void *), int32_t (*hashEqualsKey)(void *, void *),
		void (*destructKeys)(void *), void (*destructValues)(void *)) {
	stHash *hash = mallocLocal(sizeof(stHash));
	hash->hash = create_hashtable(0, hashKey, hashEqualsKey, destructKeys, destructValues);
	hash->destructKeys = destructKeys != NULL;
	hash->destructValues = destructValues != NULL;
	return hash;
}

void st_hash_destruct(stHash *hash) {
	hashtable_destroy(hash->hash, hash->destructValues, hash->destructKeys);
	free(hash);
}

void st_hash_insert(stHash *hash, void *key, void *value) {
	hashtable_insert(hash->hash, key, value);
}

void *st_hash_search(stHash *hash, void *key) {
	return hashtable_search(hash->hash, key);
}

void *st_hash_remove(stHash *hash, void *key) {
	return hashtable_remove(hash->hash, key, 0);
}

int32_t st_hash_size(stHash *hash) {
	return hashtable_count(hash->hash);
}

stHash_Iterator *st_hash_getIterator(stHash *hash) {
	return hashtable_iterator(hash->hash);
}

void *st_hash_getNext(stHash_Iterator *iterator) {
	if(iterator->e != NULL) {
		void *o = hashtable_iterator_key(iterator);
		hashtable_iterator_advance(iterator);
		return o;
	}
	return NULL;
}

stHash_Iterator *st_hash_copyIterator(stHash_Iterator *iterator) {
	stHash_Iterator *iterator2 = mallocLocal(sizeof(stHash_Iterator));
	iterator2->h = iterator->h;
	iterator2->e = iterator->e;
	iterator2->parent = iterator->parent;
	iterator2->index = iterator->index;
	return iterator2;
}

void st_hash_destructIterator(stHash_Iterator *iterator) {
	free(iterator);
}
