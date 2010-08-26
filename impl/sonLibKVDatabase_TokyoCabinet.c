/*
 * sonLibKVDatabase_TokyoCabinet.c
 *
 *  Created on: 18-Aug-2010
 *      Author: benedictpaten
 */

//Database functions

#include "sonLibGlobalsInternal.h"
#include "sonLibKVDatabasePrivate.h"
#include <tcutil.h>
#include <tcbdb.h>

static int database_constructP(const char *vA1, int size1, const char *vA2,
        int size2, void *a) {
    assert(size1 == sizeof(int64_t));
    assert(size2 == sizeof(int64_t));
    assert(a == NULL);
    int64_t i = *(int64_t *) vA1;
    int64_t j = *(int64_t *) vA2;
    return i - j > 0 ? 1 : (i < j ? -1 : 0);
}

static TCBDB *constructDB(const char *url) {
    int32_t ecode = mkdir(url, S_IRWXU);
    st_logInfo(
            "Tried to create the base disk directory with exit value: %i\n",
            ecode);
    char *databaseName = stString_print("%s/%s", url, "data");
    TCBDB *database;
    database = tcbdbnew();
    tcbdbsetcmpfunc(database, database_constructP, NULL);
    if (!tcbdbopen(database, databaseName, BDBOWRITER | BDBOCREAT)) {
        ecode = tcbdbecode(database);
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Opening database: %s with error: %s\n", databaseName, tcbdberrmsg(
                        ecode));
    }
    free(databaseName);
    return database;
}

static void destructDB(TCBDB *database) {
    if (!tcbdbclose(database)) {
        int32_t ecode = tcbdbecode(database);
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Closing database error: %s\n",
                tcbdberrmsg(ecode));
    }
    tcbdbdel(database);
}

static void deleteDB(TCBDB *database, const char *url) {
    destructDB(database);
    int32_t i = st_system("rm -rf %s", url);
    if (i != 0) {
        st_errAbort(
                "Tried to delete the temporary cactus disk: %s with exit value %i\n",
                url, i);
    }
}

static void writeRecord(TCBDB *database, bool applyCompression, int64_t key,
        const void *value, int64_t sizeOfRecord) {
    if (!tcbdbput(database, &key, sizeof(int64_t), value, sizeOfRecord)) {
        int32_t ecode = tcbdbecode(database);
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Writing key/value to database error: %s\n", tcbdberrmsg(ecode));
    }
}

static int64_t numberOfRecords(TCBDB *database) {
    return tcbdbrnum(database);
}

static void *getRecord(TCBDB *database, bool applyCompression, int64_t key) {
    //Return value must be freed.
    int32_t i; //the size is ignored
    return tcbdbget(database, &key, sizeof(int64_t), &i);
}

static void removeRecord(TCBDB *database, int64_t key) {
    if (!tcbdbout(database, &key, sizeof(int64_t))) {
        int32_t ecode = tcbdbecode(database);
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Removing key/value to database error: %s\n",
                tcbdberrmsg(ecode));
    }
}

static void startTransaction(TCBDB *database) {
    if (!tcbdbtranbegin(database)) {
        int32_t ecode = tcbdbecode(database);
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Tried to start a transaction but got error: %s\n",
                tcbdberrmsg(ecode));
    }
}

static void commitTransaction(TCBDB *database) {
    //Commit the transaction..
    if (!tcbdbtrancommit(database)) {
        int32_t ecode = tcbdbecode(database);
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                "Tried to commit a transaction but got error: %s\n",
                tcbdberrmsg(ecode));
    }
}

//initialisation function

void stKVDatabase_initialise_tokyoCabinet(stKVDatabase *database) {
    //Initialise the database..
    database->database = constructDB(stKVDatabase_getURL(database));
    //iterate through all the databases in the DB disk..
    database->destruct = (void(*)(void *)) destructDB;
    database->delete = (void(*)(void *, const char *)) deleteDB;
    database->writeRecord = (void(*)(void *, bool, int64_t, const void *,
            int64_t)) writeRecord;
    database->numberOfRecords = (int64_t(*)(void *)) numberOfRecords;
    database->getRecord = (void *(*)(void *, bool, int64_t)) getRecord;
    database->removeRecord = (void(*)(void *, int64_t)) removeRecord;
    database->startTransaction = (void(*)(void *)) startTransaction;
    database->commitTransaction = (void(*)(void *)) commitTransaction;
}

