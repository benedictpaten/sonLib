/*
 * sonLibKVDatabaseTest.c
 *
 */

#include "sonLibGlobalsTest.h"

static const char *url = "testDatabase";
bool applyCompression = 0;
static stKVDatabase *database;

static void teardown() {
    if (database != NULL) {
        stKVDatabase_deleteFromDisk(database);
        stKVDatabase_destruct(database);
        database = NULL;
    }
}

static void setup() {
    teardown();
    database = stKVDatabase_construct(url, applyCompression);
}

static void test_stKVDatabase_constructDestructAndDelete(CuTest *testCase) {
    setup();
    //The setup and teardown functions exercise all the three named functions.
    teardown();
}

static void test_stKVDatabase_getURL(CuTest *testCase) {
    setup();
    CuAssertStrEquals(testCase, url, stKVDatabase_getURL(database));
    teardown();
}

static void test_stKVDatabase_readWriteAndRemoveRecords(CuTest *testCase) {
    setup();
    stKVDatabase_startTransaction(database);

    CuAssertIntEquals(testCase, 0, stKVDatabase_getNumberOfRecords(database));
    //Write some records
    stKVDatabase_writeRecord(database, 1, "Red", sizeof(char) * 4);
    stKVDatabase_writeRecord(database, 2, "Green", sizeof(char) * 6);
    stKVDatabase_writeRecord(database, 0, "Blue", sizeof(char) * 5);
    stKVDatabase_writeRecord(database, 0, "Black", sizeof(char) * 6);

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
        CuAssertTrue(testCase, 0);
    }stCatch(except) {
        CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
    }
    stTryEnd

    teardown();
}

static void test_stKVDatabase_testTransactions(CuTest *testCase) {
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

    stKVDatabase_writeRecord(database, 1, "Red", sizeof(char) * 4);
    stKVDatabase_writeRecord(database, 2, "Green", sizeof(char) * 6);
    stKVDatabase_writeRecord(database, 0, "Black", sizeof(char) * 5);
    stKVDatabase_commitTransaction(database);
    //Try committing the transaction twice
    stTry {
        stKVDatabase_commitTransaction(database);
        CuAssertTrue(testCase, 0);
    }stCatch(except) {
        CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
    }
    stTryEnd

    stKVDatabase_destruct(database);

    //Now recreate the database and check it is as expected:
    database = stKVDatabase_construct(url, applyCompression);
    CuAssertIntEquals(testCase, 3, stKVDatabase_getNumberOfRecords(database));
    CuAssertStrEquals(testCase, "Red", stKVDatabase_getRecord(database, 1));
    CuAssertStrEquals(testCase, "Green", stKVDatabase_getRecord(database, 2));
    CuAssertStrEquals(testCase, "Black", stKVDatabase_getRecord(database, 0));

    teardown();
}


static void test_stKVDatabase_readWriteAndRemoveRecordsBig(CuTest *testCase) {
    setup();
    for(int32_t i=0; i<100; i++) {
        stKVDatabase_startTransaction(database);

        //Make a big old list of records..
        stSortedSet *set = stSortedSet_construct3((int (*)(const void *, const void *))stIntTuple_cmpFn, (void (*)(void *))stIntTuple_destruct);
        while(st_random() > 0.0001) {
            int32_t key = st_randomInt(0, 10000);
            stIntTuple *tuple = stIntTuple_construct(1, key);
            stSortedSet_insert(set, tuple);
            stKVDatabase_writeRecord(database, key, &key, sizeof(int32_t));
        }

        //Check they are all in there..
        CuAssertIntEquals(testCase, stSortedSet_size(set), stKVDatabase_getNumberOfRecords(database));
        stSortedSetIterator *it = stSortedSet_getIterator(set);
        stIntTuple *tuple;
        while((tuple = stSortedSet_getNext(it)) != NULL) {
            CuAssertIntEquals(testCase, stIntTuple_getPosition(tuple, 0), *(int32_t *)stKVDatabase_getRecord(database, stIntTuple_getPosition(tuple, 0)));
        }
        stSortedSet_destructIterator(it);

        //Try optionally committing the transaction and reloading the database..
        if(st_random() > 0.5) {
            stKVDatabase_commitTransaction(database);
            stKVDatabase_destruct(database);
            database = stKVDatabase_construct(url, 0);
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
            } stTryEnd
        }
        stSortedSet_destructIterator(it);
        CuAssertIntEquals(testCase, 0, stKVDatabase_getNumberOfRecords(database));

        stSortedSet_destruct(set);
        stKVDatabase_commitTransaction(database);
    }
    teardown();
}

CuSuite* sonLib_stKVDatabaseTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_stKVDatabase_getURL);
    SUITE_ADD_TEST(suite, test_stKVDatabase_readWriteAndRemoveRecords);
    SUITE_ADD_TEST(suite, test_stKVDatabase_readWriteAndRemoveRecordsBig);
    SUITE_ADD_TEST(suite, test_stKVDatabase_testTransactions);
    SUITE_ADD_TEST(suite, test_stKVDatabase_constructDestructAndDelete);
    return suite;
}

