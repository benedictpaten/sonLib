/*
 * sonLibKVDatabase.c
 *
 *  Created on: 18-Aug-2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsInternal.h"
#include "sonLibKVDatabasePrivate.h"
#include "sonLibString.h"

const char *ST_KV_DATABASE_EXCEPTION_ID = "ST_KV_DATABASE_EXCEPTION";

stKVDatabase *stKVDatabase_construct(stKVDatabaseConf *conf, bool create) {
    stKVDatabase *database = st_calloc(1, sizeof(struct stKVDatabase));
    database->conf = stKVDatabaseConf_constructClone(conf);
    database->transactionStarted = 0;
    database->deleted = 0;

    switch (stKVDatabaseConf_getType(conf)) {
    case stKVDatabaseTypeTokyoCabinet:
        stKVDatabase_initialise_tokyoCabinet(database, conf, create);
        break;
    case stKVDatabaseTypeMySql:
#ifdef HAVE_MYSQL
        stKVDatabase_initialise_MySql(database, conf, create);
#else
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "requested MySQL database, however sonlib is not compiled with MySql support");
#endif
        break;
    default:
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "BUG: unrecognized database type");
    }
    return database;
}

void stKVDatabase_destruct(stKVDatabase *database) {
    if (!database->deleted) {
        database->destruct(database);
    }
    stKVDatabaseConf_destruct(database->conf);
    free(database);
}

void stKVDatabase_deleteFromDisk(stKVDatabase *database) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to delete a database that has already been deleted\n");
    }
    database->delete(database);
    database->deleted = 1;
}

void stKVDatabase_insertRecord(stKVDatabase *database, int64_t key,
        const void *value, int64_t sizeOfRecord) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to insert a record into a database that has been deleted\n");
    }
    if (!database->transactionStarted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Tried to insert a record, but no transaction has been started\n");
    }
    database->insertRecord(database, key, value, sizeOfRecord);
}

void stKVDatabase_updateRecord(stKVDatabase *database, int64_t key,
        const void *value, int64_t sizeOfRecord) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to udpade a record in a database that has been deleted\n");
    }
    if (!database->transactionStarted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Tried to udapte a record, but no transaction has been started\n");
    }
    database->updateRecord(database, key, value, sizeOfRecord);
}

int64_t stKVDatabase_getNumberOfRecords(stKVDatabase *database) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to get the number of records from a database that has been deleted\n");
    }
    return database->numberOfRecords(database);
}

void *stKVDatabase_getRecord(stKVDatabase *database, int64_t key) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to get a record from a database that has already been deleted\n");
    }
    return database->getRecord(database, key);
}

void *stKVDatabase_getRecord2(stKVDatabase *database, int64_t key, int64_t *recordSize) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to get a record from a database that has already been deleted\n");
    }
    return database->getRecord2(database, key, recordSize);
}

void *stKVDatabase_getPartialRecord(stKVDatabase *database, int64_t key, int64_t zeroBasedByteOffset, int64_t sizeInBytes) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                    "Trying to get a record from a database that has already been deleted\n");
    }
    return database->getPartialRecord(database, key, zeroBasedByteOffset, sizeInBytes);
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
    database->removeRecord(database, key);
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
    database->startTransaction(database);
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
    database->commitTransaction(database);
    database->transactionStarted = 0;
}

stKVDatabaseConf *stKVDatabase_getConf(stKVDatabase *database) {
    return database->conf;
}
