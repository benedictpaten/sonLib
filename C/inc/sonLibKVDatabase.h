#ifndef SONLIB_KV_DATABASE_H_
#define SONLIB_KV_DATABASE_H_

#include "sonLibTypes.h"

//The exception string
extern const char *ST_KV_DATABASE_EXCEPTION_ID;

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//Database functions
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

/*
 * Constructs a non-relational database object, using the given configuration
 * information to connect to the database.  Create the database if create is
 * true, otherwise it must exist.
 */
stKVDatabase *stKVDatabase_construct(stKVDatabaseConf *conf, bool create);

/*
 * Destructs a database. If the destruction occurs during a transaction the transaction
 * is aborted and any changes are not committed to the database.
 * Also destroys any databases associated the the database.
 */
void stKVDatabase_destruct(stKVDatabase *database);

/*
 * Removes the database from the disk. Any further operations on the database will
 * create an exception, so you should generally destruct the object after calling this function.
 */
void stKVDatabase_deleteFromDisk(stKVDatabase *database);

/*
 * Add a new a key/value record into the table. Throws an exception if unsuccessful.
 */
void stKVDatabase_insertRecord(stKVDatabase *database, int64_t key, const void *value, int64_t sizeOfRecord);

/*
 * Update an existing key/value record in the table. Throws an exception if unsuccessful.
 */
void stKVDatabase_updateRecord(stKVDatabase *database, int64_t key, const void *value, int64_t sizeOfRecord);

/*
 * Returns number of records in database.
 */
int64_t stKVDatabase_getNumberOfRecords(stKVDatabase *database);

/*
 * Gets a record from the database, given the key. The record is in newly allocated memory, and must be freed.
 * Returns NULL if the database does not contain the given record.
 */
void *stKVDatabase_getRecord(stKVDatabase *database, int64_t key);

/*
 * Removes a record from the database. Throws an exception if unsuccessful.
 */
void stKVDatabase_removeRecord(stKVDatabase *database, int64_t key);

/*
 * Starts a transaction with the database. Throws an exception if unsuccessful.
 */
void stKVDatabase_startTransaction(stKVDatabase *database);

/*
 * Commits the transaction to the database. Throws an exception if unsuccessful.
 */
void stKVDatabase_commitTransaction(stKVDatabase *database);

/*
 * get the configuration object for the database.
 */
stKVDatabaseConf *stKVDatabase_getConf(stKVDatabase *database);

#endif
