/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

//Cache functions

#include "sonLibGlobalsInternal.h"

struct stCache {
    stSortedSet *cache;
};

typedef struct _cacheRecord {
    /*
     * A little object for storing records in the cache.
     */
    int64_t key, start, size;
    char *record;
} stCacheRecord;

static int cacheRecord_cmp(const void *a, const void *b) {
    const stCacheRecord *i = a;
    const stCacheRecord *j = b;
    if (i->key > j->key) {
        return 1;
    }
    if (i->key < j->key) {
        return -1;
    }
    if (i->start > j->start) {
        return 1;
    }
    if (i->start < j->start) {
        return -1;
    }
    return 0;
}

static void cacheRecord_destruct(stCacheRecord *i) {
    free(i->record);
    free(i);
}

static stCacheRecord getTempRecord(int64_t key, int64_t start, int64_t size) {
    stCacheRecord record;
    record.key = key;
    record.start = start;
    record.size = size;
    record.record = NULL;
    return record;
}

static stCacheRecord *cacheRecord_construct(int64_t key,
        const void *value, int64_t start, int64_t size, bool copyMemory) {
    assert(value != NULL);
    assert(size >= 0);
    assert(start >= 0);
    stCacheRecord *record = st_malloc(sizeof(stCacheRecord));
    record->key = key;
    record->start = start;
    record->size = size;
    record->record = copyMemory ? memcpy(st_malloc(size), value, size)
            : (char *) value;
    return record;
}

static stCacheRecord *getLessThanOrEqualRecord(stCache *cache,
        int64_t key, int64_t start, int64_t size) {
    stCacheRecord record = getTempRecord(key, start, size);
    return stSortedSet_searchLessThanOrEqual(cache->cache, &record);
}

static stCacheRecord *getGreaterThanOrEqualRecord(stCache *cache,
        int64_t key, int64_t start, int64_t size) {
    stCacheRecord record = getTempRecord(key, start, size);
    return stSortedSet_searchGreaterThanOrEqual(cache->cache, &record);
}

static bool recordContainedIn(stCacheRecord *record, int64_t key,
        int64_t start, int64_t size) {
    /*
     * Returns non zero if the record is contained in the given range.
     */
    return record->key == key && record->start >= start && record->start
            + record->size <= start + size;
}

static bool recordOverlapsWith(stCacheRecord *record, int64_t key,
        int64_t start, int64_t size) {
    /*
     * Returns non zero if the record overlaps with the given range.
     */
    if (record->key != key) {
        return 0;
    }
    if (record->start >= start) {
        return record->start < start + size;
    }
    return record->start + record->size > start;
}

static bool recordsAdjacent(stCacheRecord *record1, stCacheRecord *record2) {
    /*
     * Returns non-zero if the records abut with record1 immediately before record2.
     */
    assert(cacheRecord_cmp(record1, record2) <= 0);
    assert(!recordOverlapsWith(record1, record2->key, record2->start, record2->size));
    assert(!recordContainedIn(record1, record2->key, record2->start, record2->size));
    return record1->key == record2->key && record1->start + record1->size
            == record2->start;
}

static stCacheRecord *mergeRecords(stCacheRecord *record1,
        stCacheRecord *record2) {
    /*
     * Merges two adjacenct records.
     */
    assert(recordsAdjacent(record1, record2));
    int64_t i = record1->size + record2->size;
    char *j = memcpy(st_malloc(i), record1->record, record1->size);
    memcpy(j + record1->size, record2->record, record2->size);
    stCacheRecord *record3 = cacheRecord_construct(record1->key, j,
            record1->start, i, 0);
    return record3;
}

void deleteRecord(stCache *cache, int64_t key,
        int64_t start, int64_t size) {
    assert(!stCache_containsRecord(cache, key, start, size)); //Will not delete a record wholly contained in.
    stCacheRecord *record = getLessThanOrEqualRecord(cache, key, start,
            size);
    while (record != NULL && recordOverlapsWith(record, key, start, size)) { //could have multiple fragments in there to remove.
        if (recordContainedIn(record, key, start, size)) { //We get rid of the record because it is contained in the range
            stSortedSet_remove(cache->cache, record);
            cacheRecord_destruct(record);
            record = getLessThanOrEqualRecord(cache, key, start, size);
        } else { //The range overlaps with, but is not fully contained in, so we trim it..
            assert(record->start < start);
            assert(record->start + record->size > start);
            record->size = start - record->start;
            assert(record->size >= 0);
            break;
        }
    }
    record = getGreaterThanOrEqualRecord(cache, key, start, size);
    while (record != NULL && recordOverlapsWith(record, key, start, size)) { //could have multiple fragments in there to remove.
        if (recordContainedIn(record, key, start, size)) { //We get rid of the record because it is contained in the range
            stSortedSet_remove(cache->cache, record);
            cacheRecord_destruct(record);
            record = getGreaterThanOrEqualRecord(cache, key, start, size);
        } else { //The range overlaps with, but is not fully contained in, so we trim it..
            assert(record->start < start + size);
            assert(record->start > start);
            int64_t newSize = record->size - (start + size - record->start);
            int64_t newStart = start + size;
            assert(newSize >= 0);
            char *newMem = memcpy(st_malloc(newSize),
                    record->record + start + size - record->start, newSize);
            free(record->record);
            record->record = newMem;
            record->start = newStart;
            record->size = newSize;
            break; //We can break at this point as we have reached the end of the range (as the record overlapped)
        }
    }
}


/*
 * Public functions
 */

stCache *stCache_construct() {
    stCache *cache = st_malloc(sizeof(stCache));
    cache->cache = stSortedSet_construct3(cacheRecord_cmp,
            (void(*)(void *)) cacheRecord_destruct);
    return cache;
}

void stCache_destruct(stCache *cache) {
    stSortedSet_destruct(cache->cache);
    free(cache);
}

void stCache_clear(stCache *cache) {
    stSortedSet_destruct(cache->cache);
    cache->cache = stSortedSet_construct3(cacheRecord_cmp,
            (void(*)(void *)) cacheRecord_destruct);
}

void stCache_setRecord(stCache *cache, int64_t key,
       int64_t start, int64_t size,  const void *value) {
    //If the record is already contained we update a portion of it.
    assert(value != NULL);
    if (stCache_containsRecord(cache, key, start, size)) {
        stCacheRecord *record = getLessThanOrEqualRecord(cache, key,
                start, size);
        assert(record != NULL);
        assert(record->key == key);
        assert(record->start <= start);
        assert(record->start + record->size >= start + size);
        memcpy(record->record + start - record->start, value, size);
        return;
    }
    //Get rid of bits that are contained in this record..
    deleteRecord(cache, key, start, size);
    //Now get any left and right bits
    stCacheRecord *record1 = getLessThanOrEqualRecord(cache, key, start,
            size);
    stCacheRecord *record2 = cacheRecord_construct(key, value, start,
            size, 1);
    assert(record2 != NULL);
    if (record1 != NULL && recordsAdjacent(record1, record2)) {
        stCacheRecord *i = mergeRecords(record1, record2);
        stSortedSet_remove(cache->cache, record1);
        cacheRecord_destruct(record1);
        cacheRecord_destruct(record2);
        record2 = i;
    }
    stCacheRecord *record3 = getGreaterThanOrEqualRecord(cache, key,
            start, size);
    if (record3 != NULL && recordsAdjacent(record2, record3)) {
        stCacheRecord *i = mergeRecords(record2, record3);
        stSortedSet_remove(cache->cache, record3);
        cacheRecord_destruct(record2);
        cacheRecord_destruct(record3);
        record2 = i;
    }
    stSortedSet_insert(cache->cache, record2);
}

bool stCache_containsRecord(stCache *cache, int64_t key,
        int64_t start, int64_t size) {
    assert(start >= 0);
    assert(size >= 0);
    stCacheRecord *record = getLessThanOrEqualRecord(cache, key, start,
            size);
    if (record == NULL) {
        return 0;
    }
    if (record->key != key) {
        return 0;
    }
    assert(record->start <= start);
    if (size != INT64_MAX && start + size > record->start + record->size) { //If the record has a known length check we have all that we want.
        return 0;
    }
    return 1;
}

void *stCache_getRecord(stCache *cache, int64_t key,
        int64_t start, int64_t size, int64_t *sizeRead) {
    if (stCache_containsRecord(cache, key, start, size)) {
        stCacheRecord *record = getLessThanOrEqualRecord(cache, key,
                start, size);
        assert(record != NULL);
        int64_t j = start - record->start;
        int64_t i = size == INT64_MAX ? (record->size - j) : size;
        assert(record->start <= start);
        assert(j >= 0);
        assert(j <= record->size);
        assert(i >= 0);
        assert(j + i <= record->size);
        *sizeRead = i;
        char *cA = st_malloc(i);
        memcpy(cA, record->record + j, i);
        return cA;
    }
    return NULL;
}

bool stCache_recordsIdentical(const char *value, int64_t sizeOfRecord,
        const char *updatedValue, int64_t updatedSizeOfRecord) {
    if (sizeOfRecord != updatedSizeOfRecord) {
        return 0;
    }
    for (int64_t j = 0; j < sizeOfRecord; j++) {
        if (value[j] != updatedValue[j]) {
            return 0;
        }
    }
    return 1;
}
