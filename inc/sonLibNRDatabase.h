#ifndef SONLIB_NR_DATABASE_H_
#define SONLIB_NR_DATABASE_H_

#include "sonLibTypes.h"

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
stNRDatabase *stNRDatabase_construct(const char *url);

/*
 * Destructs a database. If the destruction occurs during a transaction the transaction
 * is aborted and any changes are not committed to the database.
 */
void stNRDatabase_destruct(stNRDatabase *database);

/*
 * Destructs the database object and also deletes the underlying database.
 * Returns non-zero for success, if unsuccessful the database object will not be destructed
 * and the method must be called again until success is achieved.
 */
bool stNRDatabse_destructAndDeleteDatabase(stNRDatabase *database);

/*
 * Returns number of tables in the database.
 */
int64_t stNRDatabase_getNumberOfTables(stNRDatabase *database);

/*
 * Gets the first table from the database, or returns NULL if empty.
 */
stNRTable *stNRDatabase_getFirstTable(stNRDatabase *database);

/*
 * Gets a table from the database, given the key.
 */
stNRTable *stNRDatabase_getTable(stNRDatabase *database, int64_t key);

/*
 * Gets an iterator over the tables in the database.
 */
stNRDatabase_tableIterator *stNRDatabase_getTableIterator(stNRDatabse *database);

/*
 * Gets the next table in the database, or NULL if empty.
 */
stNRTable *stNRDatabase_getNextTable(stNRDatabase_tableIterator *tableIterator);

/*
 * Destructs the table iterator.
 */
void stNRDatabase_destructTableIterator(stNRDatabase_tableIterator *tableIterator);

/*
 * Starts a transaction with the database. Returns non-zero for success.
 */
bool stNRDatabase_startTransaction(stNRDatabase *database);

/*
 * Commits the transaction to the database. Returns non-zero for success.
 */
bool stNRDatabase_commitTransaction(stNRDatabase *database);

/*
 * Aborts the transaction to the database. Returns non-zero for success.
 */
bool stNRDatabase_abortTransaction(stNRDatabase *database);

#endif
