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
const char *ST_KV_DATABASE_DEADLOCK_EXCEPTION_ID = "ST_KV_DATABASE_DEADLOCK_EXCEPTION";

static bool isDeadlock(stExcept *except) {
    return stExcept_idEq(except, ST_KV_DATABASE_DEADLOCK_EXCEPTION_ID);
}

stKVDatabase *stKVDatabase_construct(stKVDatabaseConf *conf, bool create) {
    stKVDatabase *database = st_calloc(1, sizeof(struct stKVDatabase));
    database->conf = stKVDatabaseConf_constructClone(conf);
    database->transactionStarted = false;
    database->deleted = false;

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
        stTry {
            database->destruct(database);
        } stCatch(ex) {
            stThrowNewCause(ex, ST_KV_DATABASE_EXCEPTION_ID, "stKVDatabase_deleteFromDisk failed");
        } stTryEnd;
    }
    stKVDatabaseConf_destruct(database->conf);
    free(database);
}

void stKVDatabase_deleteFromDisk(stKVDatabase *database) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to delete a database that has already been deleted");
    }
    stTry {
        database->delete(database);
    } stCatch(ex) {
        stThrowNewCause(ex, ST_KV_DATABASE_EXCEPTION_ID, "stKVDatabase_deleteFromDisk failed");
    } stTryEnd;
    database->deleted = true;
}

void stKVDatabase_insertRecord(stKVDatabase *database, int64_t key,
        const void *value, int64_t sizeOfRecord) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to insert a record into a database that has been deleted");
    }
    if (!database->transactionStarted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Tried to insert a record, but no transaction has been started");
    }
    stTry {
        database->insertRecord(database, key, value, sizeOfRecord);
    } stCatch(ex) {
        if (isDeadlock(ex)) {
            stThrow(ex);
        } else {
            stThrowNewCause(ex, ST_KV_DATABASE_EXCEPTION_ID, "stKVDatabase_insertRecord key %lld size %lld failed", (long long)key, (long long)sizeOfRecord);
        }
    } stTryEnd;
}

void stKVDatabase_updateRecord(stKVDatabase *database, int64_t key,
        const void *value, int64_t sizeOfRecord) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to udpade a record in a database that has been deleted");
    }
    if (!database->transactionStarted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Tried to udapte a record, but no transaction has been started");
    }
    stTry {
        database->updateRecord(database, key, value, sizeOfRecord);
    } stCatch(ex) {
        if (isDeadlock(ex)) {
            stThrow(ex);
        } else {
            stThrowNewCause(ex, ST_KV_DATABASE_EXCEPTION_ID, "stKVDatabase_updateRecord key %lld size %lld failed", (long long)key, (long long)sizeOfRecord);
        }
    } stTryEnd;
}

int64_t stKVDatabase_getNumberOfRecords(stKVDatabase *database) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to get the number of records from a database that has been deleted");
    }
    int64_t numRecs = 0;
    stTry {
        numRecs = database->numberOfRecords(database);
    } stCatch(ex) {
        if (isDeadlock(ex)) {
            stThrow(ex);
        } else {
            stThrowNewCause(ex, ST_KV_DATABASE_EXCEPTION_ID, "stKVDatabase_getNumberOfRecords failed");
        }
    } stTryEnd;
    return numRecs;
}

void *stKVDatabase_getRecord(stKVDatabase *database, int64_t key) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to get a record from a database that has already been deleted");
    }
    void *data = NULL;
    stTry {
        data = database->getRecord(database, key);
    } stCatch(ex) {
        if (isDeadlock(ex)) {
            stThrow(ex);
        } else {
            stThrowNewCause(ex, ST_KV_DATABASE_EXCEPTION_ID, "stKVDatabase_getRecord key %lld failed", (long long)key);
        }
    } stTryEnd;
    return data;
}

void *stKVDatabase_getRecord2(stKVDatabase *database, int64_t key, int64_t *recordSize) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to get a record from a database that has already been deleted");
    }
    void *data = NULL;
    stTry {
        data = database->getRecord2(database, key, recordSize);
    } stCatch(ex) {
        if (isDeadlock(ex)) {
            stThrow(ex);
        } else {
            stThrowNewCause(ex, ST_KV_DATABASE_EXCEPTION_ID, "stKVDatabase_getRecord2 key %lld failed", (long long)key);
        }
    } stTryEnd;
    return data;
}

void *stKVDatabase_getPartialRecord(stKVDatabase *database, int64_t key, int64_t zeroBasedByteOffset, int64_t sizeInBytes) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                    "Trying to get a record from a database that has already been deleted");
    }
    void *data = NULL;
    stTry {
        data = database->getPartialRecord(database, key, zeroBasedByteOffset, sizeInBytes);
    } stCatch(ex) {
        if (isDeadlock(ex)) {
            stThrow(ex);
        } else {
            stThrowNewCause(ex, ST_KV_DATABASE_EXCEPTION_ID, "stKVDatabase_getPartialRecord key %lld offset %lld size %lld failed", (long long)key, (long long)zeroBasedByteOffset, (long long)sizeInBytes);
        }
    } stTryEnd;
    return data;
}

void stKVDatabase_removeRecord(stKVDatabase *database, int64_t key) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to remove a record from a database that has already been deleted");
    }
    if (!database->transactionStarted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Tried to remove a record, but no transaction has been started");
    }
    stTry {
        database->removeRecord(database, key);
    } stCatch(ex) {
        if (isDeadlock(ex)) {
            stThrow(ex);
        } else {
            stThrowNewCause(ex, ST_KV_DATABASE_EXCEPTION_ID, "stKVDatabase_removeRecord key %lld failed", (long long)key);
        }
    } stTryEnd;
}

void stKVDatabase_startTransaction(stKVDatabase *database) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to start a transaction with a database that has already been deleted");
    }
    if (database->transactionStarted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Tried to start a transaction, but one was already started");
    }
    stTry {
        database->startTransaction(database);
    } stCatch(ex) {
        stThrowNewCause(ex, ST_KV_DATABASE_EXCEPTION_ID, "stKVDatabase_startTransaction failed");
    } stTryEnd;
    database->transactionStarted = true;
}

void stKVDatabase_commitTransaction(stKVDatabase *database) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to start a transaction with a database has already been deleted");
    }
    if (!database->transactionStarted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Tried to commit a transaction, but none was started");
    }
    stTry {
        database->commitTransaction(database);
    } stCatch(ex) {
        if (isDeadlock(ex)) {
            stThrow(ex);
        } else {
            stThrowNewCause(ex, ST_KV_DATABASE_EXCEPTION_ID, "stKVDatabase_commitTransaction failed");
        }
    } stTryEnd;
    database->transactionStarted = false;
}

void stKVDatabase_abortTransaction(stKVDatabase *database) {
    if (database->deleted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Trying to abort a transaction with a database has already been deleted");
    }
    if (database->transactionStarted) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Tried to abort a transaction, but none was started");
    }
    stTry {
        database->abortTransaction(database);
    } stCatch(ex) {
        stThrowNewCause(ex, ST_KV_DATABASE_EXCEPTION_ID, "stKVDatabase_abortTransaction failed");
    } stTryEnd;
    database->transactionStarted = false;
}

stKVDatabaseConf *stKVDatabase_getConf(stKVDatabase *database) {
    return database->conf;
}
