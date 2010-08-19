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
 * Constructs a non-relational database object, using the given url to connect to the
 * database.
 */
stKVDatabase *stKVDatabase_construct(const char *url);

/*
 * Destructs a database. If the destruction occurs during a transaction the transaction
 * is aborted and any changes are not committed to the database.
 * Also destroys any tables associated the the database.
 */
void stKVDatabase_destruct(stKVDatabase *database);

/*
 * Get the URL of the database.
 */
const char *stKVDatabase_getURL(stKVDatabase *database);

/*
 * Removes the database from the disk. Any further operations on the database will
 * create an exception, so you should generally destruct the object after calling this function.
 */
void stKVDatabase_deleteFromDisk(stKVDatabase *database);

/*
 * Returns number of tables in the database.
 */
int64_t stKVDatabase_getNumberOfTables(stKVDatabase *database);

/*
 * Gets a table from the database, given the key.
 */
stKVTable *stKVDatabase_getTable(stKVDatabase *database, const char *name);

/*
 * Starts a transaction with the database. Throws an exception if unsuccessful.
 */
void stKVDatabase_startTransaction(stKVDatabase *database);

/*
 * Commits the transaction to the database. Throws an exception if unsuccessful.
 */
void stKVDatabase_commitTransaction(stKVDatabase *database);


#endif
