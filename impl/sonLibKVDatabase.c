/*
 * sonLibKVDatabase.c
 *
 *  Created on: 18-Aug-2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsInternal.h"
#include "sonLibKVDatabasePrivate.h"

const char *ST_KV_DATABASE_EXCEPTION_ID = "ST_KV_DATABASE_EXCEPTION";

stKVDatabase *stKVDatabase_construct(const char *url, bool applyCompression) {
    //Currently just open the tokyo cabinet implementation..
    stKVDatabase *database = st_malloc(sizeof(struct stKVDatabase));
    database->url = stString_copy(url);
    database->applyCompression = applyCompression;
    database->transactionStarted = 0;
    database->deleted = 0;
    //Get type of database and fill out database methods (currently just tokyo cabinet)
    stKVDatabase_initialise_tokyoCabinet(database);
    return database;
}

void stKVDatabase_destruct(stKVDatabase *database) {
    if (!database->deleted) {
        database->destruct(database->database);
    }
    free(database->url);
    free(database);
}

const char *stKVDatabase_getURL(stKVDatabase *database) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "The database has already been deleted, so there is no URL\n");
    }
    return database->url;
}

void stKVDatabase_deleteFromDisk(stKVDatabase *database) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to delete a database that has already been deleted\n");
    }
    database->delete(database->database, stKVDatabase_getURL(database));
    database->deleted = 1;
}

void stKVDatabase_writeRecord(stKVDatabase *database, int64_t key,
        const void *value, int64_t sizeOfRecord) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to write a record into a database that has been deleted\n");
    }
    if (!database->transactionStarted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Tried to write a record, but no transaction has been started\n");
    }
    database->writeRecord(database->database, database->applyCompression, key,
            value, sizeOfRecord);
}

int64_t stKVDatabase_getNumberOfRecords(stKVDatabase *database) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to get the number of records from a database that has been deleted\n");
    }
    return database->numberOfRecords(database->database);
}

void *stKVDatabase_getRecord(stKVDatabase *database, int64_t key) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to get a record from a database that has already been deleted\n");
    }
    return database->getRecord(database->database, database->applyCompression,
            key);
}

void stKVDatabase_removeRecord(stKVDatabase *database, int64_t key) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to remove a record from a database that has already been deleted\n");
    }
    if (!database->transactionStarted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Tried to remove a record, but no transaction has been started\n");
    }
    database->removeRecord(database->database, key);
}

void stKVDatabase_startTransaction(stKVDatabase *database) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to start a transaction with a database that has already been deleted\n");
    }
    if (database->transactionStarted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Tried to start a transaction, but one was already started\n");
    }
    database->startTransaction(database->database);
    database->transactionStarted = 1;
}

void stKVDatabase_commitTransaction(stKVDatabase *database) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to start a transaction with a database has already been deleted\n");
    }
    if (!database->transactionStarted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Tried to commit a transaction, but none was started\n");
    }
    database->commitTransaction(database->database);
    database->transactionStarted = 0;
}
