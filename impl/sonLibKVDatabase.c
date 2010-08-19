/*
 * sonLibKVDatabase.c
 *
 *  Created on: 18-Aug-2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsInternal.h"
#include "sonLibKVDatabasePrivate.h"

const char *ST_KV_DATABASE_EXCEPTION_ID = "ST_KV_DATABASE_EXCEPTION";

stKVDatabase *stKVDatabase_construct(const char *url) {
    //Currently just open the tokyo cabinet implementation..
    stKVDatabase *database = st_malloc(sizeof(struct stKVDatabase));
    database->url = stString_copy(url);
    //Get type of database and fill out table methods (currently just tokyo cabinet)
    stKVDatabase_initialise_tokyoCabinet(database);
    return database;
}

void stKVDatabase_destruct(stKVDatabase *database) {
    database->destruct(database->database);
    free(database->url);
    free(database);
}

const char *stKVDatabase_getURL(stKVDatabase *database) {
    return database->url;
}

void stKVDatabase_deleteFromDisk(stKVDatabase *database) {
    database->delete(database->database);
}

int64_t stKVDatabase_getNumberOfTables(stKVDatabase *database) {
    return database->numberOfTables(database->database);
}

stKVTable *stKVDatabase_getTable(stKVDatabase *database, const char *name) {
    return database->getTable(database->database, name);
}

void stKVDatabase_startTransaction(stKVDatabase *database) {
    database->startTransaction(database->database);
}

void stKVDatabase_commitTransaction(stKVDatabase *database) {
    database->commitTransaction(database->database);
}
