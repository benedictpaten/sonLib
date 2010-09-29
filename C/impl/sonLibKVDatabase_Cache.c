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
    int64_t key, length, size;
    void *record;
} DiskCacheRecord;

int diskCacheRecord_cmp(const void *a, const void *b) {
    const DiskCacheRecord *i = a;
    const DiskCacheRecord *j = b;
    return i - j;
}

void diskCacheRecord_destruct(DiskCacheRecord *i) {
    free(i->record);
    free(i);
}

static void *diskCache_getRecord(DiskCache *diskCache, int64_t key,
        int64_t start, int64_t size, int64_t *sizeRead) {
    return NULL;
}

static bool diskCache_containsRecord(DiskCache *diskCache, int64_t key,
        int64_t start, int64_t size) {
    return 0;
}

static void diskCache_insertRecord(DiskCache *diskCache, int64_t key,
        const void *record, int64_t start, int64_t size) {
}

static void diskCache_updateRecord(DiskCache *diskCache, int64_t key,
        const void *record, int64_t start, int64_t size) {
}

static void diskCache_deleteRecord(DiskCache *diskCache, int64_t key,
        int64_t start, int64_t size) {
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
    if (diskCache_containsRecord(diskCache, key, 0, INT64_MAX)) {
        diskCache_updateRecord(diskCache, key, value, 0, sizeOfRecord);
    }
    diskCache->updateRecord(database, key, value,
            sizeOfRecord);
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
    if (diskCache_containsRecord(diskCache, key, 0, INT64_MAX)) {
        diskCache_deleteRecord(diskCache, key, 0, INT64_MAX);
    }
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

