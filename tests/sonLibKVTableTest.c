/*
 * sonLibKVTableTest.c
 *
 */

#include "sonLibGlobalsTest.h"

static const char *url = "hello";
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
    database = stKVDatabase_construct(url);
}

static void test_stKVTable_constructAndDestruct(CuTest *testCase) {
    setup();
    //Check we can not create a table without starting a transaction
    stTry {
        stKVTable_construct("one", database, 0);
        CuAssertTrue(testCase, 0);
    }stCatch(except) {
        CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_TABLE_EXCEPTION_ID);
    }
    stTryEnd stKVDatabase_startTransaction( database); //need to start a transaction to create tables etc.
    //Now construct an actual table
    stKVTable *table = stKVTable_construct("one", database, 0);
    //Check we can not create it twice
    stTry {
        stKVTable_construct("one", database, 0);
        CuAssertTrue(testCase, 0);
    }stCatch(except) {
        CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_TABLE_EXCEPTION_ID);
    }
    stTryEnd stKVTable_destruct( table);
    teardown();
}

static void test_stKVTable_getName(CuTest *testCase) {
    setup();
    stKVDatabase_startTransaction(database);
    stKVTable *table = stKVTable_construct("one", database, 0);
    CuAssertStrEquals(testCase, "one", stKVTable_getName(table));
    teardown();
}

static void test_stKVTable_getDatabase(CuTest *testCase) {
    setup();
    stKVDatabase_startTransaction(database);
    stKVTable *table1 = stKVTable_construct("one", database, 0);
    CuAssertPtrEquals(testCase, database, stKVTable_getDatabase(table1));
    teardown();
}

static void test_stKVTable_removeFromDatabase(CuTest *testCase) {
    setup();
    stKVDatabase_startTransaction(database);
    stKVTable *table = stKVTable_construct("one", database, 0);
    stKVTable_removeFromDatabase(table);
    //Check that the object now throws an exception if we try and do anything..
    stTry {
        stKVTable_getDatabase(table);
        CuAssertTrue(testCase, 0);
    }stCatch(except) {
        CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_TABLE_EXCEPTION_ID);
    }
    stTryEnd stKVTable_destruct( table);
    //Now check we have no problems constructing the table again
    stKVTable_construct("one", database, 0);

    teardown();
}

static void test_stKVTable_readWriteAndRemoveRecords(CuTest *testCase) {
    setup();
    stKVDatabase_startTransaction(database);
    stKVTable *table = stKVTable_construct("one", database, 0);

    CuAssertIntEquals(testCase, 0, stKVTable_getNumberOfRecords(table));
    //Write some records
    stKVTable_writeRecord(table, 1, "Red", sizeof(char) * 4);
    stKVTable_writeRecord(table, 2, "Green", sizeof(char) * 6);
    stKVTable_writeRecord(table, 0, "Blue", sizeof(char) * 5);
    stKVTable_writeRecord(table, 0, "Black", sizeof(char) * 6);

    //Now read the records
    CuAssertIntEquals(testCase, 3, stKVTable_getNumberOfRecords(table));
    CuAssertStrEquals(testCase, "Red", stKVTable_getRecord(table, 1));
    CuAssertStrEquals(testCase, "Green", stKVTable_getRecord(table, 2));
    CuAssertStrEquals(testCase, "Black", stKVTable_getRecord(table, 0));

    //Now try removing the records
    stKVTable_removeRecord(table, 0);
    CuAssertIntEquals(testCase, 2, stKVTable_getNumberOfRecords(table));
    CuAssertPtrEquals(testCase, NULL, stKVTable_getRecord(table, 0));
    //Test we get exception if we remove twice.
    stTry {
        stKVTable_removeRecord(table, 0);
        CuAssertTrue(testCase, 0);
    }stCatch(except) {
        CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_TABLE_EXCEPTION_ID);
    }
    stTryEnd

    teardown();
}

static void test_stKVTable_readWriteAndRemoveRecordsBig(CuTest *testCase) {
    setup();
    for(int32_t i=0; i<100; i++) {
        stKVDatabase_startTransaction(database);
        stKVTable *table = stKVTable_construct("one", database, 0);

        //Make a big old list of records..
        stSortedSet *set = stSortedSet_construct3((int (*)(const void *, const void *))stIntTuple_cmpFn, (void (*)(void *))stIntTuple_destruct);
        while(st_random() > 0.0001) {
            int32_t key = st_randomInt(0, 10000);
            stIntTuple *tuple = stIntTuple_construct(1, key);
            stSortedSet_insert(set, tuple);
            stKVTable_writeRecord(table, key, tuple, sizeof(void *));
        }

        //Check they are all in there..
        CuAssertIntEquals(testCase, stSortedSet_size(set), stKVTable_getNumberOfRecords(table));
        stSortedSetIterator *it = stSortedSet_getIterator(set);
        stIntTuple *tuple;
        while((tuple = stSortedSet_getNext(it)) != NULL) {
            CuAssertPtrEquals(testCase, tuple, stKVTable_getRecord(table, stIntTuple_getPosition(tuple, 0)));
        }
        stSortedSet_destructIterator(it);

        //Try optionally committing the transaction and reloading the database..
        if(st_random() > 0.5) {
            stKVDatabase_commitTransaction(database);
            stKVDatabase_destruct(database);
            database = stKVDatabase_construct(url);
            table = stKVDatabase_getTable(database, "one");
        }

        //Now remove each one..
        it = stSortedSet_getIterator(set);
        while((tuple = stSortedSet_getNext(it)) != NULL) {
            stKVTable_removeRecord(table, stIntTuple_getPosition(tuple, 0));
            //Test we get exception if we remove twice.
            stTry {
                stKVTable_removeRecord(table, stIntTuple_getPosition(tuple, 0));
                CuAssertTrue(testCase, 0);
            }stCatch(except) {
                CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_TABLE_EXCEPTION_ID);
            } stTryEnd
        }
        stSortedSet_destructIterator(it);
        CuAssertIntEquals(testCase, 0, stKVTable_getNumberOfRecords(table));

        stKVTable_removeFromDatabase(table);
        stKVTable_destruct(table);
        stSortedSet_destruct(set);
    }
    teardown();
}

CuSuite* sonLib_stKVTableTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_stKVTable_getName);
    SUITE_ADD_TEST(suite, test_stKVTable_getDatabase);
    SUITE_ADD_TEST(suite, test_stKVTable_removeFromDatabase);
    SUITE_ADD_TEST(suite, test_stKVTable_readWriteAndRemoveRecords);
    SUITE_ADD_TEST(suite, test_stKVTable_readWriteAndRemoveRecordsBig);
    SUITE_ADD_TEST(suite, test_stKVTable_constructAndDestruct);
    return suite;
}

