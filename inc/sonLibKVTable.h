#ifndef CACTUS_KV_TABLE_H_
#define CACTUS_KV_TABLE_H_

#include "sonLibTypes.h"

//The exception string
extern const char *ST_KV_TABLE_EXCEPTION_ID;

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
stKVTable *stKVTable_construct(const char *name, stKVDatabase *database, bool applyCompression);

/*
 * Destructs the table, but does not remove the table from the database.
 */
void stKVTable_destruct(stKVTable *table);

/*
 * Gets the name of the table.
 */
const char *stKVTable_getName(stKVTable *table);

/*
 * Returns the database containing the table.
 */
stKVDatabase *stKVTable_getDatabase(stKVTable *table);

/*
 * Removes the table from the database. Any further operations on the table object will
 * create an exception, so you should generally destruct the object after calling this function.
 */
void stKVTable_removeFromDatabase(stKVTable *table);

/*
 * Writes a key value record to the table. Throws an exception if unsuccessful.
 */
void stKVTable_writeRecord(stKVTable *table, int64_t key, const void *value, int64_t sizeOfRecord);

/*
 * Returns number of records in table.
 */
int64_t stKVTable_getNumberOfRecords(stKVTable *table);

/*
 * Gets a record from the table, given the key. The record is in newly allocated memory, and must be freed.
 * Returns NULL if the table does not contain the given record.
 */
void *stKVTable_getRecord(stKVTable *table, int64_t key);

/*
 * Removes a record from the table. Throws an exception if unsuccessful.
 */
void stKVTable_removeRecord(stKVTable *table, int64_t key);

#endif
