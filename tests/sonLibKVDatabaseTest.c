/*
 * sonLibKVDatabaseTest.c
 *
 */

#include "sonLibGlobalsTest.h"

static stKVDatabase *database;
static const char *url = "hello";

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

static void test_stKVDatabase_getNumberOfTables(CuTest *testCase) {
    setup();
    CuAssertIntEquals(testCase, 0, stKVDatabase_getNumberOfTables(database));
    stKVDatabase_startTransaction(database); //need to start a transaction to create tables etc.
    stKVTable_construct("one", database, 0);
    CuAssertIntEquals(testCase, 1, stKVDatabase_getNumberOfTables(database));
    stKVTable_construct("two", database, 1);
    CuAssertIntEquals(testCase, 2, stKVDatabase_getNumberOfTables(database));
    stKVTable_construct("three", database, 0);
    CuAssertIntEquals(testCase, 3, stKVDatabase_getNumberOfTables(database));
    teardown();
}

static void test_stKVDatabase_getTable(CuTest *testCase) {
    setup();

    CuAssertPtrEquals(testCase, NULL, stKVDatabase_getTable(database, "one"));

    stKVDatabase_startTransaction(database); //need to start a transaction to create tables etc.
    stKVTable *table1 = stKVTable_construct("one", database, 0);
    stKVTable *table2 = stKVTable_construct("two", database, 0);
    stKVTable *table3 = stKVTable_construct("three", database, 1);

    CuAssertPtrEquals(testCase, table1, stKVDatabase_getTable(database, "one"));
    CuAssertPtrEquals(testCase, table2, stKVDatabase_getTable(database, "two"));
    CuAssertPtrEquals(testCase, table3,
            stKVDatabase_getTable(database, "three"));
    CuAssertPtrEquals(testCase, NULL, stKVDatabase_getTable(database, "four"));

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

    stKVTable_construct("one", database, 0);
    stKVTable_construct("two", database, 0);
    stKVTable_construct("three", database, 1);
    stKVDatabase_commitTransaction(database);
    //Try committing the transaction twice
    stTry {
        stKVDatabase_commitTransaction(database);
        CuAssertTrue(testCase, 0);
    }stCatch(except) {
        CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
    }
    stTryEnd

    stKVDatabase_destruct( database);

    //Now recreate the database and check it is as expected:
    database = stKVDatabase_construct(url);
    CuAssertIntEquals(testCase, 3, stKVDatabase_getNumberOfTables(database));
    CuAssertStrEquals(testCase, "one", stKVTable_getName(stKVDatabase_getTable(
            database, "one")));
    CuAssertStrEquals(testCase, "two", stKVTable_getName(stKVDatabase_getTable(
            database, "two")));
    CuAssertStrEquals(testCase, "three", stKVTable_getName(
            stKVDatabase_getTable(database, "three")));
    CuAssertTrue(testCase, NULL == stKVDatabase_getTable(database, "four"));

    teardown();
}

CuSuite* sonLib_stKVDatabaseTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_stKVDatabase_getURL);
    SUITE_ADD_TEST(suite, test_stKVDatabase_getNumberOfTables);
    SUITE_ADD_TEST(suite, test_stKVDatabase_getTable);
    SUITE_ADD_TEST(suite, test_stKVDatabase_testTransactions);
    SUITE_ADD_TEST(suite, test_stKVDatabase_constructDestructAndDelete);

    return suite;
}

