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
#ifdef HAVE_KYOTO_TYCOON
#include <ktremotedb.h>
#include <kclangc.h>
#include "sonLibGlobalsInternal.h"
#include "sonLibKVDatabasePrivate.h"

using namespace std;
using namespace kyototycoon;


// the default expiration time: negative means indefinite, I believe
int64_t XT = kc::INT64MAX;

/*
 * construct in the Kyoto Tycoon case means connect to the remote DB
*/
static RemoteDB *constructDB(stKVDatabaseConf *conf, bool create) {

    // we actually do need a local DB dir for Kyoto Tycoon to store the sequences file
    const char *dbDir = stKVDatabaseConf_getDir(conf);
    mkdir(dbDir, S_IRWXU); // just let open of database generate error (FIXME: would be better to make this report errors)
    char *databaseName = stString_print("%s/%s", dbDir, "data");

    const char *dbRemote_Host = stKVDatabaseConf_getHost(conf);
    unsigned dbRemote_Port = stKVDatabaseConf_getPort(conf);
    int timeout = stKVDatabaseConf_getTimeout(conf);

    // create the database object
    RemoteDB *rdb = new RemoteDB();

    // tcrdb open sets the host and port for the rdb object
    if (!rdb->open(dbRemote_Host, dbRemote_Port, timeout)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Opening connection to host: %s with error: %s", dbRemote_Host, rdb->error().name());
    }
    free(databaseName);
    return rdb;
}

/* closes the remote DB connection and deletes the rdb object, but does not destroy the 
remote database */
static void destructDB(stKVDatabase *database) {
    RemoteDB *rdb = (RemoteDB*)database->dbImpl;
    if (rdb != NULL) {

        // close the connection: first try a graceful close, then a forced close
        if (!rdb->close(true)) {
            if (!rdb->close(false)) {
                stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Closing database error: %s",rdb->error().name());
            }
        }
        // delete the local in-memory object
        delete rdb; 
        database->dbImpl = NULL;
    }
}

/* WARNING: removes all records from the remote database */
static void deleteDB(stKVDatabase *database) {
    RemoteDB *rdb = (RemoteDB *)database->dbImpl;
    if (rdb != NULL) {
        rdb->clear();
    }
    destructDB(database);
    // this removes all records from the remove database object
}


/* check if a record already exists */
static bool recordExists(RemoteDB *rdb, int64_t key) {
    size_t sp;
    if (rdb->get((char *)&key, (size_t)sizeof(key), &sp, NULL) == NULL) {
        return false;
    } else {
        return true;
    }
}

static bool containsRecord(stKVDatabase *database, int64_t key) {
    return recordExists((RemoteDB *)database->dbImpl, key);
}

static void insertRecord(stKVDatabase *database, int64_t key, const void *value, int64_t sizeOfRecord) {
    RemoteDB *rdb = (RemoteDB *)database->dbImpl;

    size_t sizeOfKey = sizeof(int64_t);
    // add method: If the key already exists the record will not be modified and it'll return false 
    if (!rdb->add((char *)&key, sizeOfKey, (const char *)value, sizeOfRecord)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Inserting key/value to database error: %s", rdb->error().name());
    }
}

static void insertInt64(stKVDatabase *database, int64_t key, int64_t value) {
    RemoteDB *rdb = (RemoteDB *)database->dbImpl;

    // Normalize a 64-bit number in the native order into the network byte order.
    // little endian (our x86 linux machine) to big Endian....
    int64_t KCSafeIV = kyotocabinet::hton64(value);

    if (!rdb->add((char *)&key, sizeof(int64_t), (const char *)&KCSafeIV, sizeof(int64_t))) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Inserting int64 key/value to database error: %s", rdb->error().name());
    }
}

static void updateInt64(stKVDatabase *database, int64_t key, int64_t value) {
    RemoteDB *rdb = (RemoteDB *)database->dbImpl;

    // Normalize a 64-bit number in the native order into the network byte order.
    // little endian (our x86 linux machine) to big Endian....
    uint64_t KCSafeIV = kyotocabinet::hton64(value);

    if (!rdb->replace((char *)&key, sizeof(int64_t), (const char *)&KCSafeIV, sizeof(int64_t))) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Updating int64 key/value to database error: %s", rdb->error().name());
    }
}

static void updateRecord(stKVDatabase *database, int64_t key, const void *value, int64_t sizeOfRecord) {
    RemoteDB *rdb = (RemoteDB *)database->dbImpl;
    // replace method: If the key doesn't already exist it won't be created, and we'll get an error
    if (!rdb->replace((char *)&key, (size_t)sizeof(int64_t), (const char *)value, sizeOfRecord)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Updating key/value to database error: %s", rdb->error().name());
    }
}

static void setRecord(stKVDatabase *database, int64_t key, const void *value, int64_t sizeOfRecord) {
    RemoteDB *rdb = (RemoteDB *)database->dbImpl;
    if (!rdb->set((char *)&key, (size_t)sizeof(int64_t), (const char *)value, sizeOfRecord)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "kyoto tycoon setting key/value failed: %s", rdb->error().name());
    }
}

/* increment a record by the specified numerical value: atomic operation */
/* return the new record value */
static int64_t incrementInt64(stKVDatabase *database, int64_t key, int64_t incrementAmount) {
    RemoteDB *rdb = (RemoteDB *)database->dbImpl;
    int64_t returnValue = kyotocabinet::INT64MIN;

    size_t sizeOfKey = sizeof(int64_t);

    if ( (returnValue = rdb->increment((char *)&key, sizeOfKey, incrementAmount, kyotocabinet::INT64MIN, XT)) == kyotocabinet::INT64MIN ) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "kyoto tycoon incremement record failed: %s", rdb->error().name());
    }

    return returnValue;
}

// sets a bulk list of records atomically 
static void bulkSetRecords(stKVDatabase *database, stList *records) {

    RemoteDB *rdb = (RemoteDB *)database->dbImpl;
    map<string,string> recs;


    // copy the records from our C data structure to the CPP map needed for the Tycoon API
    for(int32_t i=0; i<stList_length(records); i++) {
        stKVDatabaseBulkRequest *request = (stKVDatabaseBulkRequest *)stList_get(records, i);
        recs.insert(pair<string,string>(
           string((const char *)&(request->key), sizeof(int64_t)), 
           string((const char *)request->value, request->size))
        );
    }

    // test for empty list   
    if (recs.empty()) {
        return;
    } 

    // set values, atomic = true

     
    int retVal; 
    if ((retVal = rdb->set_bulk(recs, XT, true)) < 1) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "kyoto tycoon set bulk record failed: %s", rdb->error().name());
    }

    //printf("size of insert: %d\n", retVal);
}

// remove a bulk list atomically 
static void bulkRemoveRecords(stKVDatabase *database, stList *records) {

    RemoteDB *rdb = (RemoteDB *)database->dbImpl;
    vector<string> keys;

    for(int32_t i=0; i<stList_length(records); i++) {
        int64_t key = stInt64Tuple_getPosition((stInt64Tuple *)stList_get(records, i), 0);
        keys.push_back(string((const char *)&key, sizeof(int64_t)));
    }
    // test for empty list   
    if (keys.empty()) {
        return;
    } 


    if (rdb->remove_bulk(keys, true) < 1) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "kyoto tycoon bulk remove record failed: %s", rdb->error().name());
    }
}

static int64_t numberOfRecords(stKVDatabase *database) {
    RemoteDB *rdb = (RemoteDB *)database->dbImpl;
    return rdb->count();
}

static void *getRecord2(stKVDatabase *database, int64_t key, int64_t *recordSize) {
    RemoteDB *rdb = (RemoteDB *)database->dbImpl;
    //Return value must be freed.
    size_t i;
    char *record = rdb->get((char *)&key, (size_t)sizeof(int64_t), &i, NULL);
    *recordSize = (int64_t)i;
    return record;
}

/* get a single non-string record */
static void *getRecord(stKVDatabase *database, int64_t key) {
    int64_t i;
    return getRecord2(database, key, &i);
}

/* get a single non-string record */
static int64_t getInt64(stKVDatabase *database, int64_t key) {
    RemoteDB *rdb = (RemoteDB *)database->dbImpl;

    size_t sp;
    char *record = rdb->get((char *)&key, sizeof(int64_t), &sp, NULL);

    // convert from KC native big-endian back to little-endian Intel...
    return kyotocabinet::ntoh64(*((int64_t*)record));
}

/* get part of a string record */
static void *getPartialRecord(stKVDatabase *database, int64_t key, int64_t zeroBasedByteOffset, int64_t sizeInBytes, int64_t recordSize) {
    int64_t recordSize2;
    char *record = (char *)getRecord2(database, key, &recordSize2);
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
    RemoteDB *rdb = (RemoteDB *)database->dbImpl;
    if (!rdb->remove((char *)&key, (size_t)sizeof(int64_t))) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Removing key/value to database error: %s", rdb->error().name());
    }
}


void stKVDatabase_initialise_kyotoTycoon(stKVDatabase *database, stKVDatabaseConf *conf, bool create) {
    database->dbImpl = constructDB(stKVDatabase_getConf(database), create);
    database->destruct = destructDB;
    database->deleteDatabase = deleteDB;
    database->containsRecord = containsRecord;
    database->insertRecord = insertRecord;
    database->insertInt64 = insertInt64;
    database->updateRecord = updateRecord;
    database->updateInt64 = updateInt64;
    database->setRecord = setRecord;
    database->incrementInt64 = incrementInt64;
    database->bulkSetRecords = bulkSetRecords;
    database->bulkRemoveRecords = bulkRemoveRecords;
    database->numberOfRecords = numberOfRecords;
    database->getRecord = getRecord;
    database->getInt64 = getInt64;
    database->getRecord2 = getRecord2;
    database->getPartialRecord = getPartialRecord;
    database->removeRecord = removeRecord;
}

#endif
