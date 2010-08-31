/*
 * sonLibKVDatabaseTest.c
 *
 */

#include "sonLibGlobalsTest.h"

static stKVDatabase *database = NULL;

static void teardown() {
    if (database != NULL) {
        stKVDatabase_deleteFromDisk(database);
        stKVDatabase_destruct(database);
        database = NULL;
    }
}

static stKVDatabaseConf *getConf() {
    static stKVDatabaseConf *conf = NULL;
    if (conf == NULL) {
#if 1
        conf = stKVDatabaseConf_constructTokyoCabinet("testTCDatabase");
#else
        conf = stKVDatabaseConf_constructMySql("kolossus-10", 0, "cactus", "cactus", "cactus", "cactusDbTest");
#endif
    }
    return conf;
}

static void setup() {
    teardown();
    database = stKVDatabase_construct(getConf(), true);
}

static void constructDestructAndDelete(CuTest *testCase) {
    setup();
    //The setup and teardown functions exercise all the three named functions.
    teardown();
}

static void readWriteAndRemoveRecords(CuTest *testCase) {
    setup();
    stKVDatabase_startTransaction(database);

    CuAssertIntEquals(testCase, 0, stKVDatabase_getNumberOfRecords(database));
    //Write some records
    stKVDatabase_insertRecord(database, 1, "Red", sizeof(char) * 4);
    stKVDatabase_insertRecord(database, 2, "Green", sizeof(char) * 6);
    stKVDatabase_insertRecord(database, 0, "Black", sizeof(char) * 6);

    //Now read the records
    CuAssertIntEquals(testCase, 3, stKVDatabase_getNumberOfRecords(database));
    CuAssertStrEquals(testCase, "Red", stKVDatabase_getRecord(database, 1));
    CuAssertStrEquals(testCase, "Green", stKVDatabase_getRecord(database, 2));
    CuAssertStrEquals(testCase, "Black", stKVDatabase_getRecord(database, 0));

    //Now try removing the records
    stKVDatabase_removeRecord(database, 0);
    CuAssertIntEquals(testCase, 2, stKVDatabase_getNumberOfRecords(database));
    CuAssertPtrEquals(testCase, NULL, stKVDatabase_getRecord(database, 0));
    //Test we get exception if we remove twice.
    stTry {
        stKVDatabase_removeRecord(database, 0);
        CuAssertTrue(testCase, false);
    }stCatch(except) {
        CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
    } stTryEnd;

    stKVDatabase_commitTransaction(database);
    teardown();
}

static void testTransactions(CuTest *testCase) {
    setup();
    //The setup method starts a transaction to allow you to create tables.
    //We will try committing the transaction and then reclaiming the created

    //First create some stuff to store..
    stKVDatabase_startTransaction(database); //need to start a transaction to create tables etc.
    //Try starting the transaction again to capture exception
    stTry {
        stKVDatabase_startTransaction(database);
        CuAssertTrue(testCase, 0);
    }stCatch(except) {
        CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
    }stTryEnd

    stKVDatabase_insertRecord(database, 1, "Red", sizeof(char) * 4);
    stKVDatabase_insertRecord(database, 2, "Green", sizeof(char) * 6);
    stKVDatabase_insertRecord(database, 0, "Black", sizeof(char) * 5);
    stKVDatabase_commitTransaction(database);
    //Try committing the transaction twice
    stTry {
        stKVDatabase_commitTransaction(database);
        CuAssertTrue(testCase, 0);
    }stCatch(except) {
        CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
    } stTryEnd;

    stKVDatabase_destruct(database);

    //Now re-open the database and check it is as expected:
    database = stKVDatabase_construct(getConf(), false);
    CuAssertIntEquals(testCase, 3, stKVDatabase_getNumberOfRecords(database));
    CuAssertStrEquals(testCase, "Red", stKVDatabase_getRecord(database, 1));
    CuAssertStrEquals(testCase, "Green", stKVDatabase_getRecord(database, 2));
    CuAssertStrEquals(testCase, "Black", stKVDatabase_getRecord(database, 0));

    teardown();
}


/* Check that all tuple records in a set are present and have the expect
 * value.  The expected value in the set is multiplied by valueMult to get
 * the actual expected value */
static void readWriteAndRemoveRecordsBigCheck(CuTest *testCase, stSortedSet *set, int valueMult) {
    CuAssertIntEquals(testCase, stSortedSet_size(set), stKVDatabase_getNumberOfRecords(database));
    stSortedSetIterator *it = stSortedSet_getIterator(set);
    stIntTuple *tuple;
    while((tuple = stSortedSet_getNext(it)) != NULL) {
        int32_t *value = (int32_t *)stKVDatabase_getRecord(database, stIntTuple_getPosition(tuple, 0));
        CuAssertIntEquals(testCase, valueMult*stIntTuple_getPosition(tuple, 0), *value);
        free(value);
    }
    stSortedSet_destructIterator(it);
}

static void readWriteAndRemoveRecordsBigIteration(CuTest *testCase, int numRecords, bool reopenDatabase) {
    stKVDatabase_startTransaction(database);

    //Make a big old list of records..
    stSortedSet *set = stSortedSet_construct3((int (*)(const void *, const void *))stIntTuple_cmpFn, (void (*)(void *))stIntTuple_destruct);
    while (stSortedSet_size(set) < numRecords) {
        int32_t key = st_randomInt(0, 100*numRecords);
        stIntTuple *tuple = stIntTuple_construct(1, key);
        if (stSortedSet_search(set, tuple) == NULL) {
            stSortedSet_insert(set, tuple);
            stKVDatabase_insertRecord(database, key, &key, sizeof(int32_t));
        } else {
            stIntTuple_destruct(tuple); // already in db
        }
    }

    readWriteAndRemoveRecordsBigCheck(testCase, set, 1);

    //Update all records to negate values
    stSortedSetIterator *it = stSortedSet_getIterator(set);
    stIntTuple *tuple;
    while((tuple = stSortedSet_getNext(it)) != NULL) {
        int32_t *value = (int32_t *)stKVDatabase_getRecord(database, stIntTuple_getPosition(tuple, 0));
        *value *= -1;
        stKVDatabase_updateRecord(database, stIntTuple_getPosition(tuple, 0), value, sizeof(int32_t));
        free(value);
    }
    stSortedSet_destructIterator(it);

    readWriteAndRemoveRecordsBigCheck(testCase, set, -1);

    //Try optionally committing the transaction and reloading the database..
    if(reopenDatabase) {
        stKVDatabase_commitTransaction(database);
        stKVDatabase_destruct(database);
        database = stKVDatabase_construct(getConf(), false);
        stKVDatabase_startTransaction(database);
    }

    //Now remove each one..
    it = stSortedSet_getIterator(set);
    while((tuple = stSortedSet_getNext(it)) != NULL) {
        stKVDatabase_removeRecord(database, stIntTuple_getPosition(tuple, 0));
        //Test we get exception if we remove twice.
        stTry {
            stKVDatabase_removeRecord(database, stIntTuple_getPosition(tuple, 0));
            CuAssertTrue(testCase, 0);
        }stCatch(except) {
            CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
        } stTryEnd;
    }
    stSortedSet_destructIterator(it);
    CuAssertIntEquals(testCase, 0, stKVDatabase_getNumberOfRecords(database));

    stSortedSet_destruct(set);
    stKVDatabase_commitTransaction(database);
}

static void readWriteAndRemoveRecordsBig(CuTest *testCase) {
    setup();
    readWriteAndRemoveRecordsBigIteration(testCase, 10, false);
    readWriteAndRemoveRecordsBigIteration(testCase, 56, true);
    readWriteAndRemoveRecordsBigIteration(testCase, 123, false);
    readWriteAndRemoveRecordsBigIteration(testCase, 245, true);
    teardown();
}

static void test_stKVDatabaseConf_constructFromString_tokyoCabinet(CuTest *testCase) {
    const char *tokyoCabinetTestString = "<st_kv_database_conf type='tokyo_cabinet'><tokyo_cabinet database_dir='foo'/></st_kv_database_conf>";
    stKVDatabaseConf *conf = stKVDatabaseConf_constructFromString(tokyoCabinetTestString);
    CuAssertTrue(testCase, stKVDatabaseConf_getType(conf) == stKVDatabaseTypeTokyoCabinet);
    CuAssertStrEquals(testCase, "foo", stKVDatabaseConf_getDir(conf));
}

static void test_stKVDatabaseConf_constructFromString_mysql(CuTest *testCase) {
#ifndef HAVE_MYSQL
    return;
#endif
    const char *tokyoCabinetTestString = "<st_kv_database_conf type='mysql'><mysql host='enormous' port='5' user='foo' password='bar' database_name='mammals' table_name='flowers'/></st_kv_database_conf>";
    stKVDatabaseConf *conf = stKVDatabaseConf_constructFromString(tokyoCabinetTestString);
    CuAssertTrue(testCase, stKVDatabaseConf_getType(conf) == stKVDatabaseTypeMySql);
    CuAssertTrue(testCase, stKVDatabaseConf_getDir(conf) == NULL);
    CuAssertStrEquals(testCase, "enormous", stKVDatabaseConf_getHost(conf));
    CuAssertIntEquals(testCase, 5, stKVDatabaseConf_getPort(conf));
    CuAssertStrEquals(testCase, "foo", stKVDatabaseConf_getUser(conf));
    CuAssertStrEquals(testCase, "bar", stKVDatabaseConf_getPassword(conf));
    CuAssertStrEquals(testCase, "mammals", stKVDatabaseConf_getDatabaseName(conf));
    CuAssertStrEquals(testCase, "flowers", stKVDatabaseConf_getTableName(conf));
}

CuSuite* sonLib_stKVDatabaseTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, readWriteAndRemoveRecords);
    SUITE_ADD_TEST(suite, readWriteAndRemoveRecordsBig);
    SUITE_ADD_TEST(suite, testTransactions);
    SUITE_ADD_TEST(suite, constructDestructAndDelete);
    SUITE_ADD_TEST(suite, test_stKVDatabaseConf_constructFromString_tokyoCabinet);
    SUITE_ADD_TEST(suite, test_stKVDatabaseConf_constructFromString_mysql);
    return suite;
}

