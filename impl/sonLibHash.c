/*
 * sonLibHash.c
 *
 *  Created on: 4 Apr 2010
 *      Author: benedictpaten
 */
#include "sonLibGlobalsPrivate.h"

static uint32_t st_hash_key( const void *k ) {
	return (uint32_t)(size_t)k;
}

static int st_hash_equalKey( const void *key1, const void *key2 ) {
	return key1 == key2;
}

st_Hash *stHash_construct() {
	return stHash_construct3(st_hash_key, st_hash_equalKey, NULL, NULL);
}

st_Hash *stHash_construct2(void (*destructKeys)(void *), void (*destructValues)(void *)) {
	return stHash_construct3(st_hash_key, st_hash_equalKey, destructKeys, destructValues);
}

st_Hash *stHash_construct3(uint32_t (*hashKey)(const void *), int (*hashEqualsKey)(const void *, const void *),
		void (*destructKeys)(void *), void (*destructValues)(void *)) {
	st_Hash *hash = st_malloc(sizeof(st_Hash));
	hash->hash = create_hashtable(0, hashKey, hashEqualsKey, destructKeys, destructValues);
	hash->destructKeys = destructKeys != NULL;
	hash->destructValues = destructValues != NULL;
	return hash;
}

void st_hash_destruct(st_Hash *hash) {
	hashtable_destroy(hash->hash, hash->destructValues, hash->destructKeys);
	free(hash);
}

void st_hash_insert(st_Hash *hash, void *key, void *value) {
	hashtable_insert(hash->hash, key, value);
}

void *st_hash_search(st_Hash *hash, void *key) {
	return hashtable_search(hash->hash, key);
}

void *st_hash_remove(st_Hash *hash, void *key) {
	return hashtable_remove(hash->hash, key, 0);
}

int32_t st_hash_size(st_Hash *hash) {
	return hashtable_count(hash->hash);
}

st_HashIterator *st_hash_getIterator(st_Hash *hash) {
	return hashtable_iterator(hash->hash);
}

void *st_hash_getNext(st_HashIterator *iterator) {
	if(iterator->e != NULL) {
		void *o = hashtable_iterator_key(iterator);
		hashtable_iterator_advance(iterator);
		return o;
	}
	return NULL;
}

st_HashIterator *st_hash_copyIterator(st_HashIterator *iterator) {
	st_HashIterator *iterator2 = st_malloc(sizeof(st_HashIterator));
	iterator2->h = iterator->h;
	iterator2->e = iterator->e;
	iterator2->parent = iterator->parent;
	iterator2->index = iterator->index;
	return iterator2;
}

void st_hash_destructIterator(st_HashIterator *iterator) {
	free(iterator);
}

st_List *st_hash_getKeys(st_Hash *hash) {
	st_List *list = st_list_construct();
	st_HashIterator *iterator = st_hash_getIterator(hash);
	void *item;
	while((item = st_hash_getNext(iterator)) != NULL) {
		st_list_append(list, item);
	}
	st_hash_destructIterator(iterator);
	return list;
}

st_List *st_hash_getValues(st_Hash *hash) {
	st_List *list = st_list_construct();
	st_HashIterator *iterator = st_hash_getIterator(hash);
	void *item;
	while((item = st_hash_getNext(iterator)) != NULL) {
		st_list_append(list, st_hash_search(hash, item));
	}
	st_hash_destructIterator(iterator);
	return list;
}
