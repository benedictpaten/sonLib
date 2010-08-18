#ifndef CACTUS_NR_TABLE_H_
#define CACTUS_NR_TABLE_H_

#include "sonLibTypes.h"

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//Database table functions
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

/*
 * Constructs a table in the database. If applyCompression is non-zero then
 * records added to the table are compressed and decompressed as they are accessed.
 */
stNRTable *stNRTable_construct(int64_t key, stNRDatabase *database, bool applyCompression);

/*
 * Destructs the table, but does not remove the table from the database.
 */
void stNRTable_destruct(stNRTable *table);

/*
 * Gets the key associated with the table.
 */
int64_t stNRTable_getKey(stNRTable *table);

/*
 * Destructs the table also removing the table from the database. Returns non-zero
 * for success, if unsuccessful the database object will not be destructed and
 * the method must be called again until success is achieved.
 */
bool stNRTable_destructAndRemoveFromDatabase(stNRTable *table);

/*
 * Returns number of records in table.
 */
int64_t stNRTable_getNumberOfRecords(stNRTable *table);

/*
 * Gets a record from the table, given the key. The record is in newly allocated memory, and must be freed.
 * Returns NULL if the table does not contain the given record.
 */
void *stNRTable_getRecord(stNRTable *table, int64_t key);

/*
 * Writes a key value record to the table. Returns non-zero if successful.
 */
bool stNRTable_writeRecord(stNRTable *database, int64_t key, const void *value, int64_t sizeOfRecord);

/*
 * Removes a record from the table. Returns non-zero if successful.
 */
bool stNRTable_removeRecord(stNRTable *database, int64_t key);

/*
 * Constructs an iterator over the keys of a table's records.
 */
stNRTable_recordIterator *stNRTable_getIterator(stNRTable *table);

/*
 * Gets the next key from the iterator, returning INT64_MAX when exhausted.
 */
int64_t stNRTable_getNextKey(stNRTable_recordIterator *iterator);

/*
 * Destructs the table iterator.
 */
void stNRTable_destructIterator(stNRTable_recordIterator *iterator);


#endif
