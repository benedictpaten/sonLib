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

static int keyCmp(const char *vA1, int size1, const char *vA2,
        int size2, void *a) {
    assert(size1 == sizeof(int64_t));
    assert(size2 == sizeof(int64_t));
    assert(a == NULL);
    int64_t i = *(int64_t *) vA1;
    int64_t j = *(int64_t *) vA2;
    return i - j > 0 ? 1 : (i < j ? -1 : 0);
}

static TCBDB *constructDB(stKVDatabaseConf *conf, bool create) {
    const char *dbDir = stKVDatabaseConf_getDir(conf);
    mkdir(dbDir, S_IRWXU); // just let open of database generate error (FIXME: would be better to make this report errors)
    char *databaseName = stString_print("%s/%s", dbDir, "data");
    TCBDB *dbImpl = tcbdbnew();
    tcbdbsetcmpfunc(dbImpl, keyCmp, NULL);
    unsigned opts = BDBOWRITER | (create ? BDBOCREAT|BDBOTRUNC : 0);
    if (!tcbdbopen(dbImpl, databaseName, opts)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Opening database: %s with error: %s", databaseName, tcbdberrmsg(tcbdbecode(dbImpl)));
    }
    free(databaseName);
    return dbImpl;
}

static void destructDB(stKVDatabase *database) {
    TCBDB *dbImpl = database->dbImpl;
    if (dbImpl != NULL) {
        if (!tcbdbclose(dbImpl)) {
            stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Closing database error: %s", tcbdberrmsg(tcbdbecode(dbImpl)));
        }
        tcbdbdel(dbImpl);
        database->dbImpl = NULL;
    }
}

static void deleteDB(stKVDatabase *database) {
    destructDB(database);
    const char *dbDir = stKVDatabaseConf_getDir(stKVDatabase_getConf(database));
    int32_t i = st_system("rm -rf %s", dbDir);
    if (i != 0) {
        st_errAbort("Tried to delete the temporary cactus disk: %s with exit code %d", dbDir, i);
    }
}

/* check if a record already exists */
static bool recordExists(TCBDB *dbImpl, int64_t key) {
    return tcbdbvnum(dbImpl, &key, sizeof(int64_t)) > 0;
}

static void insertRecord(stKVDatabase *database, int64_t key, const void *value, int64_t sizeOfRecord) {
    TCBDB *dbImpl = database->dbImpl;
    if (recordExists(dbImpl, key)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Attempt to insert a key in the database that already exists: %lld", (long long)key);
    }
    if (!tcbdbput(dbImpl, &key, sizeof(int64_t), value, sizeOfRecord)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Inserting key/value to database error: %s", tcbdberrmsg(tcbdbecode(dbImpl)));
    }
}

static void updateRecord(stKVDatabase *database, int64_t key, const void *value, int64_t sizeOfRecord) {
    TCBDB *dbImpl = database->dbImpl;
    if (!recordExists(dbImpl, key)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Attempt to update a key in the database that doesn't exists: %lld", (long long)key);
    }
    if (!tcbdbput(dbImpl, &key, sizeof(int64_t), value, sizeOfRecord)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Updating key/value to database error: %s", tcbdberrmsg(tcbdbecode(dbImpl)));
    }
}

static int64_t numberOfRecords(stKVDatabase *database) {
    TCBDB *dbImpl = database->dbImpl;
    return tcbdbrnum(dbImpl);
}

static void *getRecord(stKVDatabase *database, int64_t key) {
    TCBDB *dbImpl = database->dbImpl;
    //Return value must be freed.
    int32_t i; //the size is ignored
    return tcbdbget(dbImpl, &key, sizeof(int64_t), &i);
}

static void removeRecord(stKVDatabase *database, int64_t key) {
    TCBDB *dbImpl = database->dbImpl;
    if (!tcbdbout(dbImpl, &key, sizeof(int64_t))) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Removing key/value to database error: %s", tcbdberrmsg(tcbdbecode(dbImpl)));
    }
}

static void startTransaction(stKVDatabase *database) {
    TCBDB *dbImpl = database->dbImpl;
    if (!tcbdbtranbegin(dbImpl)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Tried to start a transaction but got error: %s", tcbdberrmsg(tcbdbecode(dbImpl)));
    }
}

static void commitTransaction(stKVDatabase *database) {
    TCBDB *dbImpl = database->dbImpl;
    //Commit the transaction..
    if (!tcbdbtrancommit(dbImpl)) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "Tried to commit a transaction but got error: %s", tcbdberrmsg(tcbdbecode(dbImpl)));
    }
}

//initialisation function

void stKVDatabase_initialise_tokyoCabinet(stKVDatabase *database, stKVDatabaseConf *conf, bool create) {
    database->dbImpl = constructDB(stKVDatabase_getConf(database), create);
    database->destruct = destructDB;
    database->delete = deleteDB;
    database->insertRecord = insertRecord;
    database->updateRecord = updateRecord;
    database->numberOfRecords = numberOfRecords;
    database->getRecord = getRecord;
    database->removeRecord = removeRecord;
    database->startTransaction = startTransaction;
    database->commitTransaction = commitTransaction;
}

