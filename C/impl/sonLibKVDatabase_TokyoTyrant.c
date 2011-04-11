/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibKVDatabase_TokyoTyrant.c
 *
 *  Created on: 4-8-11
 *      Author: epaull
 */

//Database functions

#include "sonLibGlobalsInternal.h"
#include "sonLibKVDatabasePrivate.h"

#ifdef HAVE_TOKYO_TYRANT
#include <tcutil.h>
#include <tcrdb.h>

static int keyCmp(const char *vA1, int size1, const char *vA2,
        int size2, void *a) {
    assert(size1 == sizeof(int64_t));
    assert(size2 == sizeof(int64_t));
    assert(a == NULL);
    int64_t i = *(int64_t *) vA1;
    int64_t j = *(int64_t *) vA2;
    return i - j > 0 ? 1 : (i < j ? -1 : 0);
}

/*
* construct in the Tokyo Tyrant case means connect to the remote DB
*/
static TCRDB *constructDB(stKVDatabaseConf *conf, bool create) {
    const char *dbRemote_Host = stKVDatabaseConf_getHost(conf);
    const char *dbRemote_Port = stKVDatabaseConf_getPort(conf);
    TCRDB *rdb = tcrdbnew();
    unsigned opts = BDBOWRITER | (create ? BDBOCREAT|BDBOTRUNC : 0);
    if (!tcrdbopen(rdb, dbRemote_Host, dbRemote_Port)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Opening database: %s with error: %s", dbRemote_Host, tcrdberrmsg(tcrdbecode(rdb)));
    }
    return rdb;
}


/* closes the remote DB connection and deletes the rdb object */
static void destructDB(stKVDatabase *database) {
    TCRDB *rdb = database->dbImpl;
    if (rdb != NULL) {
        if (!tcrdbclose(rdb)) {
            stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Closing database error: %s", tcrdberrmsg(tcrdbecode(rdb)));
        }
        tcrdbdel(rdb);
        database->dbImpl = NULL;
    }
}

/* check if a record already exists */
static bool recordExists(TCRDB *rdb, int64_t key) {
    if (tcrdbget(rdb, &key, sizeof(int64_t)) == NULL) {
        return false;
    } else {
        return true;
    }
}

static bool containsRecord(stKVDatabase *database, int64_t key) {
    return recordExists(database->dbImpl, key);
}

/* uses tcrdbputkeep : if the record already exists it won't overwrite it */
static void insertRecord(stKVDatabase *database, int64_t key, const void *value, int64_t sizeOfRecord) {
    TCRDB *dbImpl = database->dbImpl;
    if (recordExists(dbImpl, key)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Attempt to insert a key in the database that already exists: %lld", (long long)key);
    }
    if (!tcrdbputkeep(dbImpl, &key, sizeof(int64_t), value, sizeOfRecord)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Inserting key/value to database error: %s", tcrdberrmsg(tcrdbecode(dbImpl)));
    }
}

/* tcrdbput will overwrite the record if it exists */
static void updateRecord(stKVDatabase *database, int64_t key, const void *value, int64_t sizeOfRecord) {
    TCRDB *dbImpl = database->dbImpl;
    if (!recordExists(dbImpl, key)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Attempt to update a key in the database that doesn't exists: %lld", (long long)key);
    }
    if (!tcrdbput(dbImpl, &key, sizeof(int64_t), value, sizeOfRecord)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Updating key/value to database error: %s", tcrdberrmsg(tcrdbecode(dbImpl)));
    }
}

static int64_t numberOfRecords(stKVDatabase *database) {
    TCRDB *dbImpl = database->dbImpl;
    return tcrdbrnum(dbImpl);
}

static void *getRecord2(stKVDatabase *database, int64_t key, int64_t *recordSize) {
    TCRDB *dbImpl = database->dbImpl;
    //Return value must be freed.
    int32_t i;
    void *record = tcrdbget(dbImpl, &key, sizeof(int64_t), &i);
    *recordSize = i;
    return record;
}

/* get a single non-string record */
static void *getRecord(stKVDatabase *database, int64_t key) {
    int64_t i;
    return getRecord2(database, key, &i);
}

/* get part of a string record */
static void *getPartialRecord(stKVDatabase *database, int64_t key, int64_t zeroBasedByteOffset, int64_t sizeInBytes, int64_t recordSize) {
    int64_t recordSize2;
    char *record = getRecord2(database, key, &recordSize2);
    if(recordSize2 != recordSize) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "The given record size is incorrect: %lld, should be %lld", (long long)recordSize, recordSize2);
    }
    if(record == NULL) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "The record does not exist: %lld for partial retrieval", (long long)key);
    }
    if(zeroBasedByteOffset < 0 || sizeInBytes < 0 || zeroBasedByteOffset + sizeInBytes > recordSize) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Partial record retrieval to out of bounds memory, record size: %lld, requested start: %lld, requested size: %lld", (long long)recordSize, (long long)zeroBasedByteOffset, (long long)sizeInBytes);
    }
    void *partialRecord = memcpy(st_malloc(sizeInBytes), record + zeroBasedByteOffset, sizeInBytes);
    free(record);
    return partialRecord;
}

static void removeRecord(stKVDatabase *database, int64_t key) {
    TCRDB *dbImpl = database->dbImpl;
    if (!tcrdbout(dbImpl, &key, sizeof(int64_t))) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Removing key/value to database error: %s", tcrdberrmsg(tcrdbecode(dbImpl)));
    }
}

static void startTransaction(stKVDatabase *database) {
    TCRDB *dbImpl = database->dbImpl;
    if (!tcrdbtranbegin(dbImpl)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Tried to start a transaction but got error: %s", tcrdberrmsg(tcrdbecode(dbImpl)));
    }
}

static void commitTransaction(stKVDatabase *database) {
    TCRDB *dbImpl = database->dbImpl;
    //Commit the transaction..
    if (!tcrdbtrancommit(dbImpl)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Tried to commit a transaction but got error: %s", tcrdberrmsg(tcrdbecode(dbImpl)));
    }
}

static void abortTransaction(stKVDatabase *database) {
    TCRDB *dbImpl = database->dbImpl;
    if (!tcrdbtranabort(dbImpl)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Tried to abort a transaction but got error: %s", tcrdberrmsg(tcrdbecode(dbImpl)));
    }
}

//initialisation function

void stKVDatabase_initialise_tokyoTyrant(stKVDatabase *database, stKVDatabaseConf *conf, bool create) {
    database->dbImpl = constructDB(stKVDatabase_getConf(database), create);
    database->destruct = destructDB;
    database->containsRecord = containsRecord;
    database->insertRecord = insertRecord;
    database->updateRecord = updateRecord;
    database->numberOfRecords = numberOfRecords;
    database->getRecord = getRecord;
    database->getRecord2 = getRecord2;
    database->getPartialRecord = getPartialRecord;
    database->removeRecord = removeRecord;
    database->startTransaction = startTransaction;
    database->commitTransaction = commitTransaction;
    database->abortTransaction = abortTransaction;
}

#endif
