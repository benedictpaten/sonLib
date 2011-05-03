/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibKVDatabase_KyotoTycoon.cpp
 *
 *  Created on: 5-1-11
 *      Author: epaull
 * 
 * Note: all the KT methods seem to have a C and a CPP version (in terms of the arguments) , 
 * and for this implementation we're using the plain C versions as much as we can.
 */

//Database functions

#include "sonLibGlobalsInternal.h"
#include "sonLibKVDatabasePrivate.h"

#ifdef HAVE_KYOTO_TYCOON
#include <ktremotedb.h>

using namespace std;
using namespace kyototycoon;

/*
* construct in the Tokyo Tyrant case means connect to the remote DB
*/
static RemoteDB *constructDB(stKVDatabaseConf *conf, bool create) {

    // we actually do need a local DB dir for Kyoto Tycoon to store the sequences file
    const char *dbDir = stKVDatabaseConf_getDir(conf);
    mkdir(dbDir, S_IRWXU); // just let open of database generate error (FIXME: would be better to make this report errors)
    char *databaseName = stString_print("%s/%s", dbDir, "data");

    const char *dbRemote_Host = stKVDatabaseConf_getHost(conf);
    unsigned dbRemote_Port = stKVDatabaseConf_getPort(conf);
    int timeout = stKVDatabaseConf_getTimeout(conf);

    // new tycoon object
    RemoteDB rdb;

    // tcrdb open sets the host and port for the rdb object
    if (!rdb.open(dbRemote_Host, dbRemote_Port, timeout)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Opening connection to host: %s with error: %s", dbRemote_Host, rdb.error().name());
    }

    free(databaseName);
    return rdb;
}

/* closes the remote DB connection and deletes the rdb object, but does not destroy the 
remote database */
static void destructDB(stKVDatabase *database) {
    RemoteDB rdb = database->dbImpl;
    if (rdb != NULL) {

        // close the connection: first try a graceful close, then a forced close
        if (!rdb.close(true)) {
            if (!rdb.close(false)) {
                stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Closing database error: %s",rdb.error().name());
            }
        }
        // delete the local in-memory object
        
        database->dbImpl = NULL;
    }
}

/* WARNING: removes all records from the remote database */
static void deleteDB(stKVDatabase *database) {
    RemoteDB rdb = database->dbImpl;
    if (rdb != NULL) {
        rdb.clear();
    }
    destructDB(database);
    // this removes all records from the remove database object
}


/* check if a record already exists */
static bool recordExists(RemoteDB rdb, int64_t key) {
    size_t sp;
    if (rdb.get(&key,sizeof(key), &sp) == NULL)
        return false;
    } else {
        return true;
    }
}

static bool containsRecord(stKVDatabase *database, int64_t key) {
    return recordExists(database->dbImpl, key);
}

static void insertRecord(stKVDatabase *database, int64_t key, const void *value, int64_t sizeOfRecord) {
    RemoteDB rdb = database->dbImpl;
    // add method: If the key already exists the record will not be modified and it'll return false 
    if (!rdb.add(&key, sizeof(int64_t), value, sizeOfRecord)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Inserting key/value to database error: %s", rdb.error().name());
    }
}

static void updateRecord(stKVDatabase *database, int64_t key, const void *value, int64_t sizeOfRecord) {
    RemoteDB rdb = database->dbImpl;
    // replace method: If the key doesn't already exist it won't be created, and we'll get an error
    if (!rdb.replace(&key, sizeof(int64_t), value, sizeOfRecord)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Updating key/value to database error: %s", rdb.error().name());
    }
}

static int64_t numberOfRecords(stKVDatabase *database) {
    RemoteDB rdb = database->dbImpl;
    return rdb.count();
}

static void *getRecord2(stKVDatabase *database, int64_t key, int64_t *recordSize) {
    RemoteDB rdb = database->dbImpl;
    //Return value must be freed.
    size_t i;
    void *record = (void *)rdb.get(&key, sizeof(int64_t), &i);
    *recordSize = (int64_t)i;
    return record;
}

/* get a single non-string record */
static void *getRecord(stKVDatabase *database, int64_t key) {
    size_t i;
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
    RemoteDB rdb = database->dbImpl;
    if (!rdb.remove(&key, sizeof(int64_t))) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Removing key/value to database error: %s", rdb.error().name());
    }
}

static void startTransaction(stKVDatabase *database) {
    // transactions supported through bulk_... methods
    return;
}

static void commitTransaction(stKVDatabase *database) {
    // transactions supported through bulk_... methods
    return;
}

static void abortTransaction(stKVDatabase *database) {
    // transactions supported through bulk_... methods
    return;
}


void stKVDatabase_initialise_kyotoTycoon(stKVDatabase *database, stKVDatabaseConf *conf, bool create) {
    database->dbImpl = constructDB(stKVDatabase_getConf(database), create);
    database->destruct = destructDB;
    database->delete = deleteDB;
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
