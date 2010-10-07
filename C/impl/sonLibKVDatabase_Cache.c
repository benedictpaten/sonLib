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
    /*
     * Struct that is sandwiched within a kvDatabase object, with functions
     * to be called by the sandwich functions of the struct.
     */
    stSortedSet *cache;
    int64_t size;
    int64_t boundarySize;

    void (*destruct)(stKVDatabase *);
    void (*delete)(stKVDatabase *);
    bool (*containsRecord)(stKVDatabase *, int64_t);
    void (*insertRecord)(stKVDatabase *, int64_t, const void *, int64_t);
    void (*updateRecord)(stKVDatabase *, int64_t, const void *, int64_t);
    void *(*getRecord2)(stKVDatabase *database, int64_t key,
            int64_t *recordSize);
    void *(*getPartialRecord)(stKVDatabase *database, int64_t key,
            int64_t zeroBasedByteOffset, int64_t sizeInBytes, int64_t recordSize);
    void (*removeRecord)(stKVDatabase *, int64_t key);
} DiskCache;

typedef struct _diskCacheRecord {
    /*
     * A little object for storing records in the cache.
     */
    int64_t key, start, size;
    char *record;
} DiskCacheRecord;

static int diskCacheRecord_cmp(const void *a, const void *b) {
    const DiskCacheRecord *i = a;
    const DiskCacheRecord *j = b;
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

static void diskCacheRecord_destruct(DiskCacheRecord *i) {
    free(i->record);
    free(i);
}

static DiskCacheRecord getTempRecord(int64_t key, int64_t start, int64_t size) {
    DiskCacheRecord record;
    record.key = key;
    record.start = start;
    record.size = size;
    record.record = NULL;
    return record;
}

static DiskCacheRecord *diskCacheRecord_construct(int64_t key,
        const void *value, int64_t start, int64_t size, bool copyMemory) {
#ifdef BEN_DEBUG
    assert(value != NULL);
    assert(size >= 0);
    assert(start >= 0);
#endif
    DiskCacheRecord *record = st_malloc(sizeof(DiskCacheRecord));
    record->key = key;
    record->start = start;
    record->size = size;
    record->record = copyMemory ? memcpy(st_malloc(size), value, size)
            : (char *) value;
    return record;
}

static DiskCacheRecord *getLessThanOrEqualRecord(DiskCache *diskCache,
        int64_t key, int64_t start, int64_t size) {
    DiskCacheRecord record = getTempRecord(key, start, size);
    return stSortedSet_searchLessThanOrEqual(diskCache->cache, &record);
}

static DiskCacheRecord *getGreaterThanOrEqualRecord(DiskCache *diskCache,
        int64_t key, int64_t start, int64_t size) {
    DiskCacheRecord record = getTempRecord(key, start, size);
    return stSortedSet_searchGreaterThanOrEqual(diskCache->cache, &record);
}

static bool diskCache_containsRecord(DiskCache *diskCache, int64_t key,
        int64_t start, int64_t size) {
#ifdef BEN_DEBUG
    assert(start >= 0);
    assert(size >= 0);
#endif
    DiskCacheRecord *record = getLessThanOrEqualRecord(diskCache, key, start,
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

static void *diskCache_getRecord(DiskCache *diskCache, int64_t key,
        int64_t start, int64_t size, int64_t *sizeRead) {
    if (diskCache_containsRecord(diskCache, key, start, size)) {
        DiskCacheRecord *record = getLessThanOrEqualRecord(diskCache, key,
                start, size);
        assert(record != NULL);
        int64_t i = size == INT64_MAX ? record->size : size;
        int64_t j = start - record->start;
#ifdef BEN_DEBUG
        assert(record->start <= start);
        assert(j >= 0);
        assert(j <= record->size);
        assert(i >= 0);
        assert(j + i <= record->size);
#endif
        *sizeRead = i;
        void *o = memcpy(st_malloc(i), record->record + j, i);
        assert(o != NULL);
        return o;
    }
    return NULL;
}

static bool recordContainedIn(DiskCacheRecord *record, int64_t key,
        int64_t start, int64_t size) {
    /*
     * Returns non zero if the record is contained in the given range.
     */
    return record->key == key && record->start >= start && record->start
            + record->size <= start + size;
}

static bool recordOverlapsWith(DiskCacheRecord *record, int64_t key,
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

static void diskCache_deleteRecord(DiskCache *diskCache, int64_t key,
        int64_t start, int64_t size) {
    DiskCacheRecord *record = getLessThanOrEqualRecord(diskCache, key, start,
            size);
    while (record != NULL && recordOverlapsWith(record, key, start, size)) { //could have multiple fragments in there to remove.
        if (recordContainedIn(record, key, start, size)) { //We get rid of the record because it is contained in the range
            stSortedSet_remove(diskCache->cache, record);
            diskCacheRecord_destruct(record);
            record = getLessThanOrEqualRecord(diskCache, key, start, size);
        } else { //The range overlaps with, but is not fully contained in, so we trim it..
            assert(record->start < start);
            assert(record->start + record->size > start);
            record->size = start - record->start;
            assert(record->size >= 0);
            break;
        }
    }
    record = getGreaterThanOrEqualRecord(diskCache, key, start, size);
    while (record != NULL && recordOverlapsWith(record, key, start, size)) { //could have multiple fragments in there to remove.
        if (recordContainedIn(record, key, start, size)) { //We get rid of the record because it is contained in the range
            stSortedSet_remove(diskCache->cache, record);
            diskCacheRecord_destruct(record);
            record = getGreaterThanOrEqualRecord(diskCache, key, start, size);
        } else { //The range overlaps with, but is not fully contained in, so we trim it..
            assert(record->start < start + size);
            assert(record->start > start);
            int64_t newSize = record->size - (start + size - record->start);
            int64_t newStart = start + size;
            assert(newSize >= 0);
            char *newMem = memcpy(st_malloc(newSize), record->record + start
                    + size - record->start, newSize);
            free(record->record);
            record->record = newMem;
            record->start = newStart;
            record->size = newSize;
            break; //We can break at this point as we have reached the end of the range (as the record overlapped)
        }
    }
}

static bool recordsAdjacent(DiskCacheRecord *record1, DiskCacheRecord *record2) {
    /*
     * Returns non-zero if the records abut with record1 immediately before record2.
     */
#if BEN_DEBUG //Check the records do not overlap
    assert(diskCacheRecord_cmp(record1, record2) <= 0);
    assert(!recordOverlapsWith(record1, record2->key, record2->start, record2->size));
    assert(!recordContainedIn(record1, record2->key, record2->start, record2->size));
#endif
    return record1->key == record2->key && record1->start + record1->size
            == record2->start;
}

static DiskCacheRecord *mergeRecords(DiskCacheRecord *record1,
        DiskCacheRecord *record2) {
    /*
     * Merges two adjacenct records.
     */
#ifdef BEN_DEBUG
    assert(recordsAdjacent(record1, record2));
#endif
    int64_t i = record1->size + record2->size;
    char *j = memcpy(st_malloc(i), record1->record, record1->size);
    memcpy(j + record1->size, record2->record, record2->size);
    DiskCacheRecord *record3 = diskCacheRecord_construct(record1->key, j,
            record1->start, i, 0);
    return record3;
}

static void diskCache_insertRecord(DiskCache *diskCache, int64_t key,
        const void *value, int64_t start, int64_t size) {
    //If the record is already contained we update a portion of it.
    assert(value != NULL);
    if (diskCache_containsRecord(diskCache, key, start, size)) {
        DiskCacheRecord *record = getLessThanOrEqualRecord(diskCache, key,
                start, size);
#ifdef BEN_DEBUG
        assert(record != NULL);
        assert(record->key == key);
        assert(record->start <= start);
        assert(record->start + record->size >= start + size);
#endif
        memcpy(record->record + start - record->start, value, size);
        return;
    }
    //Get rid of bits that are contained in this record..
    diskCache_deleteRecord(diskCache, key, start, size);
    //Now get any left and right bits
    DiskCacheRecord *record1 = getLessThanOrEqualRecord(diskCache, key, start,
            size);
    DiskCacheRecord *record2 = diskCacheRecord_construct(key, value, start,
            size, 1);
    assert(record2 != NULL);
    if (record1 != NULL && recordsAdjacent(record1, record2)) {
        DiskCacheRecord *i = mergeRecords(record1, record2);
        stSortedSet_remove(diskCache->cache, record1);
        diskCacheRecord_destruct(record1);
        diskCacheRecord_destruct(record2);
        record2 = i;
    }
    DiskCacheRecord *record3 = getGreaterThanOrEqualRecord(diskCache, key,
            start, size);
    if (record3 != NULL && recordsAdjacent(record2, record3)) {
        DiskCacheRecord *i = mergeRecords(record2, record3);
        stSortedSet_remove(diskCache->cache, record3);
        diskCacheRecord_destruct(record2);
        diskCacheRecord_destruct(record3);
        record2 = i;
    }
    stSortedSet_insert(diskCache->cache, record2);
}

static void destruct(stKVDatabase *database) {
    DiskCache *diskCache = database->cache;
    diskCache->destruct(database); //cleanup the underlying database
    stSortedSet_destruct(diskCache->cache);
    free(diskCache);
}

static bool containsRecord(stKVDatabase *database, int64_t key) {
    DiskCache *diskCache = database->cache;
    assert(diskCache != NULL);
    if (diskCache_containsRecord(diskCache, key, 0, INT64_MAX)) { //only returns true if the whole record is contained in the cache.
        return 1;
    }
    return diskCache->containsRecord(database, key);
}

static void insertRecord(stKVDatabase *database, int64_t key,
        const void *value, int64_t sizeOfRecord) {
    DiskCache *diskCache = database->cache;
    diskCache->insertRecord(database, key, value, sizeOfRecord); //Call this first
    diskCache_insertRecord(diskCache, key, value, 0, sizeOfRecord);
}

static void *getRecord2(stKVDatabase *database, int64_t key,
        int64_t *recordSize) {
    DiskCache *diskCache = database->cache;
    if (diskCache_containsRecord(diskCache, key, 0, INT64_MAX)) {
        void *i = diskCache_getRecord(diskCache, key, 0, INT64_MAX, recordSize);
        assert(i != NULL);
        return i;
    }
    void *record = diskCache->getRecord2(database, key, recordSize);
    if (record != NULL) {
        diskCache_insertRecord(diskCache, key, record, 0, *recordSize);
    }
    return record;
}

static void *getRecord(stKVDatabase *database, int64_t key) {
    int64_t i;
    return getRecord2(database, key, &i);
}

static void *getPartialRecord(stKVDatabase *database, int64_t key,
        int64_t zeroBasedByteOffset, int64_t sizeInBytes, int64_t recordSize) {
    if(sizeInBytes == 0) {
        return st_malloc(0);
    }
    DiskCache *diskCache = database->cache;
    if (diskCache_containsRecord(diskCache, key, zeroBasedByteOffset,
            sizeInBytes)) {
        int64_t i;
        void *record = diskCache_getRecord(diskCache, key, zeroBasedByteOffset,
                sizeInBytes, &i);
        assert(i == sizeInBytes);
        assert(record != NULL);
        return record;
    }
    /*
     * We get a few kilobytes more/less on either side to avoid making too many queries.
     */
    int64_t i = zeroBasedByteOffset - diskCache->boundarySize;
    if (i < 0) {
        i = 0;
    }
    int64_t j = sizeInBytes + zeroBasedByteOffset - i + diskCache->boundarySize;
    if(j + i > recordSize) {
        j = recordSize - i;
    }
    char *k = diskCache->getPartialRecord(database, key, i, j, recordSize);
    diskCache_insertRecord(diskCache, key, k, i, j);
    void *record = memcpy(st_malloc(sizeInBytes), k + zeroBasedByteOffset - i, sizeInBytes);
    free(k);
    return record;
    /*void *record = diskCache->getPartialRecord(database, key,
     zeroBasedByteOffset, sizeInBytes);
     diskCache_insertRecord(diskCache, key, record, zeroBasedByteOffset,
     sizeInBytes);
     return record;*/
}

static bool recordsIdentical(const char *value, int64_t sizeOfRecord,
        const char *updatedValue, int64_t updatedSizeOfRecord) {
    if(sizeOfRecord != updatedSizeOfRecord) {
        return 0;
    }
    for(int64_t j=0; j<sizeOfRecord; j++) {
        if(value[j] != updatedValue[j]) {
            return 0;
        }
    }
    return 1;
}

static void updateRecord(stKVDatabase *database, int64_t key,
        const void *value, int64_t sizeOfRecord) {
    DiskCache *diskCache = database->cache;
    if (diskCache_containsRecord(diskCache, key, 0, INT64_MAX)) { //If the record is in the cache, and if
        //it is identical then we don't need to do anything.
        int64_t oldRecordSize;
        void *i = diskCache_getRecord(diskCache, key, 0, INT64_MAX, &oldRecordSize);
        assert(i != NULL);
        if(recordsIdentical(i, oldRecordSize, value, sizeOfRecord)) {
            free(i);
            return; //We don't need to do anything as the record is already on the disk.
        }
        free(i);
    }
    diskCache->updateRecord(database, key, value, sizeOfRecord);
    diskCache_insertRecord(diskCache, key, value, 0, sizeOfRecord);
}

static void removeRecord(stKVDatabase *database, int64_t key) {
    DiskCache *diskCache = database->cache;
    diskCache_deleteRecord(diskCache, key, 0, INT64_MAX);
    diskCache->removeRecord(database, key);
}

static void clearCache(stKVDatabase *database) {
    DiskCache *diskCache = database->cache;
    assert(diskCache != NULL);
    stSortedSet_destruct(diskCache->cache);
    diskCache->cache = stSortedSet_construct3(diskCacheRecord_cmp, (void(*)(
            void *)) diskCacheRecord_destruct);
    diskCache->size = 0;
}

void stKVDatabase_makeMemCache(stKVDatabase *database, int64_t size, int64_t boundarySize) {
    assert(database->cache == NULL);
    DiskCache *diskCache = st_malloc(sizeof(DiskCache));
    diskCache->cache = stSortedSet_construct3(diskCacheRecord_cmp, (void(*)(void *)) diskCacheRecord_destruct);
    diskCache->size = size;
    diskCache->boundarySize = boundarySize;

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
    database->clearCache = clearCache;
}

