/*
 * sonLibKVDatabase_TokyoCabinet.c
 *
 *  Created on: 18-Aug-2010
 *      Author: benedictpaten
 */

//Database functions

#include "sonLibGlobalsInternal.h"
#include "sonLibKVDatabasePrivate.h"

typedef struct _diskCache {
    stSortedSet *cache;
    int64_t size;

    void (*destruct)(stKVDatabase *);
    void (*delete)(stKVDatabase *);
    bool (*containsRecord)(stKVDatabase *, int64_t);
    void (*insertRecord)(stKVDatabase *, int64_t, const void *, int64_t);
    void (*updateRecord)(stKVDatabase *, int64_t, const void *, int64_t);
    void *(*getRecord2)(stKVDatabase *database, int64_t key, int64_t *recordSize);
    void *(*getPartialRecord)(stKVDatabase *database, int64_t key, int64_t zeroBasedByteOffset, int64_t sizeInBytes);
    void (*removeRecord)(stKVDatabase *, int64_t key);
} DiskCache;

typedef struct _diskCacheRecord {
    int64_t key, start, size;
    char *record;
} DiskCacheRecord;

int diskCacheRecord_cmp(const void *a, const void *b) {
    const DiskCacheRecord *i = a;
    const DiskCacheRecord *j = b;
    if(i->key > j->key) {
        return 1;
    }
    if(i->key < j->key) {
        return -1;
    }
    if(i->start > j->start) {
        return 1;
    }
    if(i->start < j->start) {
        return -1;
    }
    return 0;
}

void diskCacheRecord_destruct(DiskCacheRecord *i) {
    free(i->record);
    free(i);
}

static DiskCacheRecord getTempRecord(int64_t key,
        int64_t start, int64_t size) {
    DiskCacheRecord record;
    record.key = key;
    record.start = start;
    record.size = size;
    return record;
}

static DiskCacheRecord *diskCacheRecord_construct(int64_t key, const void *value, int64_t start, int64_t size) {
    DiskCacheRecord *record = st_malloc(sizeof(DiskCacheRecord));
    record->key = key;
    record->start = start;
    record->size = size;
    record->record = memcpy(st_malloc(size), value, size);
    return record;
}

static DiskCacheRecord *getLessThanOrEqualRecord(DiskCache *diskCache, int64_t key,
        int64_t start, int64_t size) {
    DiskCacheRecord record = getTempRecord(key, start, size);
    return stSortedSet_searchLessThanOrEqual(diskCache->cache, &record);
}

static DiskCacheRecord *getGreaterThanOrEqualRecord(DiskCache *diskCache, int64_t key,
        int64_t start, int64_t size) {
    DiskCacheRecord record = getTempRecord(key, start, size);
    return stSortedSet_searchGreaterThanOrEqual(diskCache->cache, &record);
}

static bool diskCache_containsRecord(DiskCache *diskCache, int64_t key,
        int64_t start, int64_t size) {
    DiskCacheRecord *record = getLessThanOrEqualRecord(diskCache, key, start, size);
    if(record == NULL) {
        return 0;
    }
    if(record->key != key) {
        return 0;
    }
    assert(record->start <= start);
    if(size != INT64_MAX && record->start + record->size < start + size) { //If the record has a known length check we have all that we want.
        return 0;
    }
    return 1;
}

static void *diskCache_getRecord(DiskCache *diskCache, int64_t key,
        int64_t start, int64_t size, int64_t *sizeRead) {
    if(diskCache_containsRecord(diskCache, key, start, size)) {
        DiskCacheRecord *record = getLessThanOrEqualRecord(diskCache, key, start, size);
        int64_t i = size == INT64_MAX ? record->size : size;
#ifdef BEN_DEBUG
        assert(record->start <= start);
        assert(i >= 0);
        assert(i + start - record->start < record->size);
#endif
        return memcpy(st_malloc(i), record->record + start - record->start, i);
    }
    return NULL;
}

static bool recordContainedIn(DiskCacheRecord *record, int64_t key, int64_t start, int64_t size) {
    return record->key == key && record->start >= start &&
            record->size + record->start - start <= size;
}

static void diskCache_deleteRecord(DiskCache *diskCache, int64_t key, int64_t start, int64_t size) {
    DiskCacheRecord *record = getLessThanOrEqualRecord(diskCache, key, start, size);
    while(record != NULL && recordContainedIn(record, key, start, size)) { //could have multiple fragments in there to remove.
        stSortedSet_remove(diskCache->cache, record);
        diskCacheRecord_destruct(record);
        record = getLessThanOrEqualRecord(diskCache, key, start, size);
    }
    record = getGreaterThanOrEqualRecord(diskCache, key, start, size);
    while(record != NULL && recordContainedIn(record, key, start, size)) { //could have multiple fragments in there to remove.
        stSortedSet_remove(diskCache->cache, record);
        diskCacheRecord_destruct(record);
        record = getGreaterThanOrEqualRecord(diskCache, key, start, size);
    }
}

static bool recordsAdjacent(DiskCacheRecord *record1, DiskCacheRecord *record2) {
    return record1->key == record2->key && record1->start + record1->size > record2->start;
}

static DiskCacheRecord *mergeRecords(DiskCacheRecord *record1, DiskCacheRecord *record2) {
    int64_t i = 0;
    char *j = NULL;
    DiskCacheRecord *record3 = diskCacheRecord_construct(record1->key, j, record1->start, i);
    return record3;
}

static void diskCache_insertRecord(DiskCache *diskCache, int64_t key,
        const void *value, int64_t start, int64_t size) {
    //If the record is already contained we update a portion of it.
    if(diskCache_containsRecord(diskCache, key, start, size)) {
        DiskCacheRecord *record = getLessThanOrEqualRecord(diskCache, key, start, size);
        memcpy(record->record, record->record + start - record->start, size);
        return;
    }
    //Get rid of bits that are contained in this record..
    diskCache_deleteRecord(diskCache, key, start, size);
    //Now get any left and right bits
    DiskCacheRecord *record1 = getLessThanOrEqualRecord(diskCache, key, start, size);
    DiskCacheRecord *record2 = diskCacheRecord_construct(key, value, start, size);
    DiskCacheRecord *record3 = getGreaterThanOrEqualRecord(diskCache, key, start, size);
    if(record1 != NULL && recordsAdjacent(record1, record2)) {
        DiskCacheRecord *i = mergeRecords(record1, record2);
                mergeRecords(record1, record2);
        stSortedSet_remove(diskCache->cache, record1);
        diskCacheRecord_destruct(record1);
        diskCacheRecord_destruct(record2);
        record2 = i;
    }
    if(record1 != NULL && recordsAdjacent(record2, record3)) {
        DiskCacheRecord *i = mergeRecords(record2, record3);
        stSortedSet_remove(diskCache->cache, record3);
        diskCacheRecord_destruct(record2);
        diskCacheRecord_destruct(record3);
        record2 = i;
    }
    stSortedSet_insert(diskCache->cache, record2);
}

static void destruct(stKVDatabase *database) {
    DiskCache *diskCache = database->dbImpl;
    stSortedSet_destruct(diskCache->cache);
    diskCache->destruct(database);
    free(diskCache);
}

static bool containsRecord(stKVDatabase *database, int64_t key) {
    DiskCache *diskCache = database->dbImpl;
    if (diskCache_containsRecord(diskCache, key, 0, INT64_MAX)) {
        return 1;
    }
    return diskCache->containsRecord(database, key);
}

static void insertRecord(stKVDatabase *database, int64_t key,
        const void *value, int64_t sizeOfRecord) {
    DiskCache *diskCache = database->dbImpl;
    diskCache->insertRecord(database, key, value,
            sizeOfRecord);
    diskCache_insertRecord(diskCache, key, value, 0, sizeOfRecord);
}

static void updateRecord(stKVDatabase *database, int64_t key,
        const void *value, int64_t sizeOfRecord) {
    DiskCache *diskCache = database->dbImpl;
    diskCache->updateRecord(database, key, value,
            sizeOfRecord);
    diskCache_insertRecord(diskCache, key, value, 0, sizeOfRecord);
}

static void *getRecord2(stKVDatabase *database, int64_t key,
        int64_t *recordSize) {
    DiskCache *diskCache = database->dbImpl;
    if (diskCache_containsRecord(diskCache, key, 0, INT64_MAX)) {
        return diskCache_getRecord(diskCache, key, 0, INT64_MAX, recordSize);
    }
    void *record = diskCache->getRecord2(database, key, recordSize);
    diskCache_insertRecord(diskCache, key, record, 0, *recordSize);
    return record;
}

static void *getRecord(stKVDatabase *database, int64_t key) {
    int64_t i;
    return getRecord2(database, key, &i);
}

static void *getPartialRecord(stKVDatabase *database, int64_t key,
        int64_t zeroBasedByteOffset, int64_t sizeInBytes) {
    DiskCache *diskCache = database->dbImpl;
    if (diskCache_containsRecord(diskCache, key, zeroBasedByteOffset, sizeInBytes)) {
        int64_t i;
        void *record = diskCache_getRecord(diskCache, key, zeroBasedByteOffset,
                sizeInBytes, &i);
        assert(i == sizeInBytes);
        return record;
    }
    void *record = diskCache->getPartialRecord(database,
            key, zeroBasedByteOffset, sizeInBytes);
    diskCache_insertRecord(diskCache, key, record, zeroBasedByteOffset, sizeInBytes);
    return record;
}

static void removeRecord(stKVDatabase *database, int64_t key) {
    DiskCache *diskCache = database->dbImpl;
    diskCache_deleteRecord(diskCache, key, 0, INT64_MAX);
    diskCache->removeRecord(database, key);
}

void stKVDatabase_makeMemCache(stKVDatabase *database, int32_t size) {
    assert(database->cache == NULL);
    DiskCache *diskCache = st_malloc(sizeof(DiskCache));
    diskCache->cache = stSortedSet_construct3(diskCacheRecord_cmp, (void (*)(void *))diskCacheRecord_destruct);

    diskCache->destruct = database->destruct;
    diskCache->containsRecord = database->containsRecord;
    diskCache->insertRecord = database->insertRecord;
    diskCache->updateRecord = database->updateRecord;
    diskCache->getRecord2 = database->getRecord2;
    diskCache->getPartialRecord = database->getPartialRecord;
    diskCache->removeRecord = database->removeRecord;

    database->cache = diskCache;
    database->destruct = destruct;
    database->containsRecord = containsRecord;
    database->insertRecord = insertRecord;
    database->updateRecord = updateRecord;
    database->getRecord = getRecord;
    database->getRecord2 = getRecord2;
    database->getPartialRecord = getPartialRecord;
    database->removeRecord = removeRecord;
}

