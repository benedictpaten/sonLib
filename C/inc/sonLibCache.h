/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

#ifndef SONLIB__DATABASE_H_
#define SONLIB__DATABASE_H_

#include "sonLibTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//Cache functions
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

/*
 *
 * The size in the number of bytes of the cache. The cache is simple, it stores
 * all records retrieved, added and updated until it is full, then it starts to
 * remove stuff that has been accessed least recently to make room for the newly added
 * stuff.
 */
stCache *stCache_construct(int64_t size);

/*
 * Destructs the cache.
 */
void stCache_destruct(stCache *cache);

/*
 * Clears the cache.
 */
void stCache_clear(stCache *cache);

/*
 * Update an existing key/value record fragment in the cache. If the record does not exist it is inserted. Throws an exception if unsuccessful.
 * The offset is the start of the record fragment.
 */
void stCache_setRecord(stCache *cache, int64_t key, int64_t zeroBasedByteOffset, int64_t sizeInBytes, const void *value);

/*
 * Returns non-zero if the cache contains all of the given record fragment. If zeroBasedByteOffset=INT64_MAX and
 * sizeInBytes=INT64_MAX then no overlap is required.
 */
bool stCache_containsRecord(stCache *cache, int64_t key, int64_t zeroBasedByteOffset, int64_t sizeInBytes);

/*
 * Gets a record from the cache, given the key. This function allows the partial retrieval of a record, using the
 * given offset and the size of the requested retrieval.
 * The function returns NULL if the desired part of the record is not in the cache exist.
 * If the sizeInBytes equals INT64_MAX then the maximal size contiguous fragment of the record is returned.
 * Record size is initialised with the size of the returned buffer.
 */
void *stCache_getRecord(stCache *cache, int64_t key, int64_t zeroBasedByteOffset, int64_t sizeInBytes, int64_t *recordSize);

void stCache_deleteRecord(stCache *cache, int64_t key,
        int64_t start, int64_t size);

bool stCache_recordsIdentical(const char *value, int64_t sizeOfRecord,
        const char *updatedValue, int64_t updatedSizeOfRecord);

#ifdef __cplusplus
}
#endif
#endif
