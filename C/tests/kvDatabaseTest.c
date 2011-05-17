/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * kVDatabaseTest.c
 *
 */

#include "sonLibGlobalsTest.h"
#include "kvDatabaseTestCommon.h"

static stKVDatabaseConf *conf = NULL;
static stKVDatabase *database = NULL;
static bool USE_CACHE = 0;
static bool CLEAR_CACHE = 0;

static void teardown() {
    if (database != NULL) {
        stKVDatabase_deleteFromDisk(database);
        stKVDatabase_destruct(database);
        database = NULL;
    }
}

static void setup() {
    teardown();
    database = stKVDatabase_construct(conf, true);
    teardown();
    database = stKVDatabase_construct(conf, true);
    if (USE_CACHE) {
        stKVDatabase_makeMemCache(database, 50000, 5); //Makes a cache with a 50k cache.
    }
}

static void constructDestructAndDelete(CuTest *testCase) {
    setup();
    //The setup and teardown functions exercise all the three named functions.
    teardown();
}

static void readWriteAndUpdateIntRecords(CuTest *testCase) {
    setup();
    CuAssertIntEquals(testCase, 0, stKVDatabase_getNumberOfRecords(database));
    //Write some int64 records
    CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, 1));
    stKVDatabase_insertInt64(database, 1, (int64_t)50);
    stKVDatabase_insertInt64(database, 2, (int64_t)100);
    stKVDatabase_insertInt64(database, 3, (int64_t)150);
    CuAssertTrue(testCase, stKVDatabase_getInt64(database, 1) == 50);
    CuAssertTrue(testCase, stKVDatabase_getInt64(database, 2) == 100);
    CuAssertTrue(testCase, stKVDatabase_getInt64(database, 3) == 150);

    // test update
    stKVDatabase_insertInt64(database, 4, (int64_t)100);
    stKVDatabase_updateInt64(database, 4, (int64_t)55);
    CuAssertTrue(testCase, stKVDatabase_getInt64(database, 4) == 55);

    
    //Now try removing a record
    stKVDatabase_removeRecord(database, 4);
    CuAssertIntEquals(testCase, 3, stKVDatabase_getNumberOfRecords(database));
    CuAssertPtrEquals(testCase, NULL, stKVDatabase_getRecord(database, 0));
    CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, 0));

    teardown();
}

static void readWriteAndRemoveRecords(CuTest *testCase) {
    setup();
    CuAssertIntEquals(testCase, 0, stKVDatabase_getNumberOfRecords(database));
    //Write some records
    CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, 1));
    stKVDatabase_insertRecord(database, 1, "Red", sizeof(char) * 4);
    CuAssertTrue(testCase, stKVDatabase_containsRecord(database, 1));

    CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, 2));
    stKVDatabase_insertRecord(database, 2, "Green", sizeof(char) * 6);
    CuAssertTrue(testCase, stKVDatabase_containsRecord(database, 2));

    CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, 0));
    stKVDatabase_insertRecord(database, 0, "Black", sizeof(char) * 6);
    CuAssertTrue(testCase, stKVDatabase_containsRecord(database, 0));

    //Now read and check the records exist
    CuAssertIntEquals(testCase, 3, stKVDatabase_getNumberOfRecords(database));
    CuAssertStrEquals(testCase, "Red", stKVDatabase_getRecord(database, 1));
    CuAssertStrEquals(testCase, "Green", stKVDatabase_getRecord(database, 2));
    CuAssertStrEquals(testCase, "Black", stKVDatabase_getRecord(database, 0));
    CuAssertTrue(testCase, stKVDatabase_containsRecord(database, 1));
    CuAssertTrue(testCase, stKVDatabase_containsRecord(database, 2));
    CuAssertTrue(testCase, stKVDatabase_containsRecord(database, 0));

    //Now check we can retrieve records partially
    CuAssertStrEquals(testCase, "d", stKVDatabase_getPartialRecord(database, 1, 2, 2, sizeof(char) * 4));
    CuAssertStrEquals(testCase, "ed", stKVDatabase_getPartialRecord(database, 1, 1, 3, sizeof(char) * 4));
    CuAssertStrEquals(testCase, "Red", stKVDatabase_getPartialRecord(database, 1, 0, 4, sizeof(char) * 4));
    char *record = stKVDatabase_getPartialRecord(database, 0, 2, 3, sizeof(char) * 6);
    record[2] = '\0';
    CuAssertStrEquals(testCase, "ac", record);

    //Now try removing the records
    stKVDatabase_removeRecord(database, 0);
    CuAssertIntEquals(testCase, 2, stKVDatabase_getNumberOfRecords(database));
    CuAssertPtrEquals(testCase, NULL, stKVDatabase_getRecord(database, 0));
    CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, 0));
    //Test we get exception if we remove twice.
    stTry {
            stKVDatabase_removeRecord(database, 0);
            CuAssertTrue(testCase, false);
        }
        stCatch(except)
            {
                CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
            }stTryEnd;

    teardown();
}

static void partialRecordRetrieval(CuTest *testCase) {
    setup();

    //Make some number of large records
    stList *records = stList_construct3(0, free);
    stList *recordSizes = stList_construct3(0, (void(*)(void *)) stIntTuple_destruct);
    for (int32_t i = 0; i < 300; i++) {
        int32_t size = st_randomInt(0, 80);
        size = size * size * size; //Use cubic size distribution
        char *randomRecord = st_malloc(size * sizeof(char));
        for (int32_t j = 0; j < size; j++) {
            randomRecord[j] = (char) st_randomInt(0, 100);
        }
        stList_append(records, randomRecord);
        stList_append(recordSizes, stIntTuple_construct(1, size));
        CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, i));
        stKVDatabase_insertRecord(database, i, randomRecord, size * sizeof(char));
        CuAssertTrue(testCase, stKVDatabase_containsRecord(database, i));
        //st_uglyf("I am creating the record %i %i\n", i, size);
    }

    if (CLEAR_CACHE) {
        stKVDatabase_clearCache(database);
    }

    while (st_random() > 0.001) {
        int32_t recordKey = st_randomInt(0, stList_length(records));
        CuAssertTrue(testCase, stKVDatabase_containsRecord(database, recordKey));

        char *record = stList_get(records, recordKey);
        int32_t size = stIntTuple_getPosition(stList_get(recordSizes, recordKey), 0);

        //Get partial record
        int32_t start = size > 0 ? st_randomInt(0, size) : 0;
        int32_t partialSize = size - start > 0 ? st_randomInt(start, size) - start : 0;
        assert(start >= 0);
        assert(partialSize >= 0);
        assert(partialSize + start <= size);
        //st_uglyf("I am getting record %i %i %i %i\n", recordKey, start, partialSize, size);
        char *partialRecord = stKVDatabase_getPartialRecord(database, recordKey, start * sizeof(char),
                partialSize * sizeof(char), size * sizeof(char));

        //Check they are equivalent..
        for (int32_t i = 0; i < partialSize; i++) {
            if (record[start + i] != partialRecord[i]) {
                st_uglyf("There was a difference %i %i for record %i %i\n", record[start + i], partialRecord[i], i,
                        partialSize);
            }
            //CuAssertTrue(testCase, record[start + i] == partialRecord[i]);
        }

        //Check we can not get out of bounds.. (start less than zero)
        stTry {
                stKVDatabase_getPartialRecord(database, recordKey, -1, 1, size * sizeof(char));
            }stCatch(except)
                {
                    CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
                }stTryEnd;

        //Check we can not get out of bounds.. (start greater than index start)
        stTry {
                stKVDatabase_getPartialRecord(database, recordKey, size, 1, size * sizeof(char));
            }stCatch(except)
                {
                    CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
                }stTryEnd;

        //Check we can not get out of bounds.. (total size if greater than record length)
        stTry {
                stKVDatabase_getPartialRecord(database, recordKey, 0, size + 1, size * sizeof(char));
            }stCatch(except)
                {
                    CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
                }stTryEnd;

        //Check we can not get non existent record
        stTry {
                stKVDatabase_getPartialRecord(database, 1000000, 0, size, size * sizeof(char));
            }stCatch(except)
                {
                    CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
                }stTryEnd;
    }

    stList_destruct(records);
    stList_destruct(recordSizes);

    teardown();
}

static void testIncrementRecord(CuTest *testCase) {
    /*
     * Tests incrementing records
     */
    setup();
    // note: record we're incrementing must be the same size (8-byte integer) as
    // the one we're adding to it
    int64_t i = 100;
    int64_t key = 7;
    stKVDatabase_insertInt64(database, key, i);
    CuAssertTrue(testCase,  stKVDatabase_getInt64(database, key) == 100);

    int64_t l = 10;
    int64_t m = 15;
    int64_t n = -25;

    CuAssertTrue(testCase, stKVDatabase_incrementInt64(database, key, l) == 110);
    CuAssertTrue(testCase, stKVDatabase_getInt64(database, key) == 110);
    CuAssertTrue(testCase, stKVDatabase_incrementInt64(database, key, m) == 125);
    CuAssertTrue(testCase, stKVDatabase_getInt64(database, key) == 125);
    CuAssertTrue(testCase, stKVDatabase_incrementInt64(database, key, n) == 100);
    CuAssertTrue(testCase, stKVDatabase_getInt64(database, key) == 100);

    teardown();
}

static void testSetRecord(CuTest *testCase) {
    /*
     * Tests 'setting' records (where you don't know if it's an update or an insert).
     */
    setup();
    int64_t i = 100;
    stKVDatabase_setRecord(database, 1, &i, sizeof(int64_t));
    int64_t *j = stKVDatabase_getRecord(database, 1);
    CuAssertTrue(testCase, i == *j);
    free(j);
    i = 110;
    stKVDatabase_setRecord(database, 1, &i, sizeof(int64_t));
    j = stKVDatabase_getRecord(database, 1);
    CuAssertTrue(testCase, i == *j);
    free(j);
    teardown();
}


static void testBulkSetRecords(CuTest *testCase) {
    /*
     * Tests doing a bulk update of a set of records.
     */
    setup();
    int64_t i = 100, j = 110, k = 120, l = 130;
    stKVDatabase_insertRecord(database, 1, &i, sizeof(int64_t));

    stList *requests = stList_construct3(0, (void(*)(void *)) stKVDatabaseBulkRequest_destruct);
    stList_append(requests, stKVDatabaseBulkRequest_constructInsertRequest(2, &j, sizeof(int64_t)));
    stList_append(requests, stKVDatabaseBulkRequest_constructSetRequest(3, &k, sizeof(int64_t)));
    stList_append(requests, stKVDatabaseBulkRequest_constructUpdateRequest(1, &l, sizeof(int64_t)));

    stKVDatabase_bulkSetRecords(database, requests);

    stList_destruct(requests);

    int64_t *m = stKVDatabase_getRecord(database, 1);
    CuAssertTrue(testCase, m != NULL);
    CuAssertTrue(testCase, l == *m);
    free(m);

    m = stKVDatabase_getRecord(database, 2);
    CuAssertTrue(testCase, m != NULL);
    CuAssertTrue(testCase, j == *m);
    free(m);

    m = stKVDatabase_getRecord(database, 3);
    CuAssertTrue(testCase, m != NULL);
    CuAssertTrue(testCase, k == *m);
    free(m);

    teardown();
}

static void testBulkRemoveRecords(CuTest *testCase) {
    /*
     * Tests doing a bulk update of a set of records.
     */
    setup();
    int64_t i = 100, j = 110, k = 120, l = 130;
    stKVDatabase_insertRecord(database, 1, &i, sizeof(int64_t));
    stKVDatabase_insertRecord(database, 2, &j, sizeof(int64_t));
    stKVDatabase_insertRecord(database, 3, &k, sizeof(int64_t));
    stKVDatabase_insertRecord(database, 4, &l, sizeof(int64_t));

    CuAssertTrue(testCase, stKVDatabase_containsRecord(database, 1));
    CuAssertTrue(testCase, stKVDatabase_containsRecord(database, 2));
    CuAssertTrue(testCase, stKVDatabase_containsRecord(database, 3));
    CuAssertTrue(testCase, stKVDatabase_containsRecord(database, 4));
    CuAssertTrue(testCase, stKVDatabase_getNumberOfRecords(database) == 4);

    stList *requests = stList_construct3(0, (void(*)(void *)) stInt64Tuple_destruct);

    stList_append(requests, stInt64Tuple_construct(1, 1));
    stList_append(requests, stInt64Tuple_construct(1, 2));
    stList_append(requests, stInt64Tuple_construct(1, 3));
    stList_append(requests, stInt64Tuple_construct(1, 4));

    stKVDatabase_bulkRemoveRecords(database, requests);

    CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, 1));
    CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, 2));
    CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, 3));
    CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, 4));
    CuAssertTrue(testCase, stKVDatabase_getNumberOfRecords(database) == 0);

    stList_destruct(requests);

    teardown();
}

/*
 * Retrieves really long records from the database.
 */
static void bigRecordRetrieval(CuTest *testCase) {
    setup();
    for (int32_t i = 0; i < 10; i++) {
        int32_t size = st_randomInt(5000000, 10000000);
        char *randomRecord = st_malloc(size * sizeof(char));
        for (int32_t j = 0; j < size; j++) {
            randomRecord[j] = (char) st_randomInt(0, 100);
        }

        //st_uglyf("I am inserting record %i %i\n", i, size);
        stKVDatabase_insertRecord(database, i, randomRecord, size * sizeof(char));

        if (CLEAR_CACHE) {
            stKVDatabase_clearCache(database);
        }
        //st_uglyf("I am creating the record %i %i\n", i, size);
        //Check they are equivalent.
        int64_t size2;
        char *randomRecord2 = stKVDatabase_getRecord2(database, i, &size2);
        CuAssertTrue(testCase, size == size2);
        for (int32_t j = 0; j < size; j++) {
            CuAssertTrue(testCase, randomRecord[j] == randomRecord2[j]);
        }
    }
    teardown();
}

/* Check that all tuple records in a set are present and have the expect
 * value.  The expected value in the set is multiplied by valueMult to get
 * the actual expected value */
static void readWriteAndRemoveRecordsLotsCheck(CuTest *testCase, stSortedSet *set, int valueMult) {
    CuAssertIntEquals(testCase, stSortedSet_size(set), stKVDatabase_getNumberOfRecords(database));
    stSortedSetIterator *it = stSortedSet_getIterator(set);
    stIntTuple *tuple;
    while ((tuple = stSortedSet_getNext(it)) != NULL) {
        int32_t *value = (int32_t *) stKVDatabase_getRecord(database, stIntTuple_getPosition(tuple, 0));
        CuAssertTrue(testCase, stKVDatabase_containsRecord(database, stIntTuple_getPosition(tuple, 0)));
        CuAssertIntEquals(testCase, valueMult*stIntTuple_getPosition(tuple, 0), *value);
        free(value);
    }
    stSortedSet_destructIterator(it);
}

static void readWriteAndRemoveRecordsLotsIteration(CuTest *testCase, int numRecords, bool reopenDatabase) {
    //Make a big old list of records..
    stSortedSet *set = stSortedSet_construct3((int(*)(const void *, const void *)) stIntTuple_cmpFn,
            (void(*)(void *)) stIntTuple_destruct);
    while (stSortedSet_size(set) < numRecords) {
        int32_t key = st_randomInt(0, 100 * numRecords);
        stIntTuple *tuple = stIntTuple_construct(1, key);
        if (stSortedSet_search(set, tuple) == NULL) {
            CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, key));
            stSortedSet_insert(set, tuple);
            stKVDatabase_insertRecord(database, key, &key, sizeof(int32_t));
            CuAssertTrue(testCase, stKVDatabase_containsRecord(database, key));
        } else {
            CuAssertTrue(testCase, stKVDatabase_containsRecord(database, key));
            stIntTuple_destruct(tuple); // already in db
        }
    }

    readWriteAndRemoveRecordsLotsCheck(testCase, set, 1);

    if (CLEAR_CACHE) {
        stKVDatabase_clearCache(database);
    }

    //Update all records to negate values
    stSortedSetIterator *it = stSortedSet_getIterator(set);
    stIntTuple *tuple;
    while ((tuple = stSortedSet_getNext(it)) != NULL) {
        int32_t *value = (int32_t *) stKVDatabase_getRecord(database, stIntTuple_getPosition(tuple, 0));
        *value *= -1;
        stKVDatabase_updateRecord(database, stIntTuple_getPosition(tuple, 0), value, sizeof(int32_t));
        CuAssertTrue(testCase, stKVDatabase_containsRecord(database, stIntTuple_getPosition(tuple, 0)));
        free(value);
    }
    stSortedSet_destructIterator(it);

    readWriteAndRemoveRecordsLotsCheck(testCase, set, -1);

    //Try optionally committing the transaction and reloading the database..
    if (reopenDatabase) {
        //stKVDatabase_commitTransaction(database);
        stKVDatabase_destruct(database);
        database = stKVDatabase_construct(conf, false);
        //stKVDatabase_startTransaction(database);
    }

    //Now remove each one..
    it = stSortedSet_getIterator(set);
    while ((tuple = stSortedSet_getNext(it)) != NULL) {
        CuAssertTrue(testCase, stKVDatabase_containsRecord(database, stIntTuple_getPosition(tuple, 0)));
        stKVDatabase_removeRecord(database, stIntTuple_getPosition(tuple, 0));
        CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, stIntTuple_getPosition(tuple, 0)));
        //Test we get exception if we remove twice.
        stTry {
                stKVDatabase_removeRecord(database, stIntTuple_getPosition(tuple, 0));
                CuAssertTrue(testCase, 0);
            }
            stCatch(except)
                {
                    CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
                }stTryEnd;
    }
    stSortedSet_destructIterator(it);
    CuAssertIntEquals(testCase, 0, stKVDatabase_getNumberOfRecords(database));

    stSortedSet_destruct(set);
}

/*
 * Tests the retrieval of lots of records.
 */
static void readWriteAndRemoveRecordsLots(CuTest *testCase) {
    setup();
    readWriteAndRemoveRecordsLotsIteration(testCase, 10, false);
    readWriteAndRemoveRecordsLotsIteration(testCase, 56, true);
    readWriteAndRemoveRecordsLotsIteration(testCase, 123, false);
    readWriteAndRemoveRecordsLotsIteration(testCase, 245, true);
    teardown();
}

static void test_stKVDatabaseConf_constructFromString_tokyoCabinet(CuTest *testCase) {
    const char *xmlTestString =
            "<st_kv_database_conf type='tokyo_cabinet'><tokyo_cabinet database_dir='foo'/></st_kv_database_conf>";
    stKVDatabaseConf *conf = stKVDatabaseConf_constructFromString(xmlTestString);
    CuAssertTrue(testCase, stKVDatabaseConf_getType(conf) == stKVDatabaseTypeTokyoCabinet);
    CuAssertStrEquals(testCase, "foo", stKVDatabaseConf_getDir(conf));
}

static void test_stKVDatabaseConf_constructFromString_mysql(CuTest *testCase) {
#ifdef HAVE_MYSQL
    const char *xmlTestString =
    "<st_kv_database_conf type='mysql'><mysql host='enormous' port='5' user='foo' password='bar' database_name='mammals' table_name='flowers'/></st_kv_database_conf>";
    stKVDatabaseConf *conf = stKVDatabaseConf_constructFromString(
            xmlTestString);
    CuAssertTrue(testCase, stKVDatabaseConf_getType(conf) == stKVDatabaseTypeMySql);
    CuAssertTrue(testCase, stKVDatabaseConf_getDir(conf) == NULL);
    CuAssertStrEquals(testCase, "enormous", stKVDatabaseConf_getHost(conf));
    CuAssertIntEquals(testCase, 5, stKVDatabaseConf_getPort(conf));
    CuAssertStrEquals(testCase, "foo", stKVDatabaseConf_getUser(conf));
    CuAssertStrEquals(testCase, "bar", stKVDatabaseConf_getPassword(conf));
    CuAssertStrEquals(testCase, "mammals", stKVDatabaseConf_getDatabaseName(conf));
    CuAssertStrEquals(testCase, "flowers", stKVDatabaseConf_getTableName(conf));
#endif
}

static void test_stKVDatabaseConf_constructFromString_postgresql(CuTest *testCase) {
#ifdef HAVE_POSTGRESQL
    const char *xmlTestString =
    "<st_kv_database_conf type='postgresql'><postgresql host='enormous' user='foo' password='bar' database_name='mammals' table_name='flowers'/></st_kv_database_conf>";
    stKVDatabaseConf *conf = stKVDatabaseConf_constructFromString(xmlTestString);
    CuAssertTrue(testCase, stKVDatabaseConf_getType(conf) == stKVDatabaseTypePostgreSql);
    CuAssertTrue(testCase, stKVDatabaseConf_getDir(conf) == NULL);
    CuAssertStrEquals(testCase, "enormous", stKVDatabaseConf_getHost(conf));
    CuAssertIntEquals(testCase, 0, stKVDatabaseConf_getPort(conf));
    CuAssertStrEquals(testCase, "foo", stKVDatabaseConf_getUser(conf));
    CuAssertStrEquals(testCase, "bar", stKVDatabaseConf_getPassword(conf));
    CuAssertStrEquals(testCase, "mammals", stKVDatabaseConf_getDatabaseName(conf));
    CuAssertStrEquals(testCase, "flowers", stKVDatabaseConf_getTableName(conf));
#endif
}

static void test_cache(CuTest *testCase) {
    /*
     * Tests all the test functions with a cache.
     * These tests do not clear the cache between the adds and the retrieves.
     */
    USE_CACHE = 1;
    readWriteAndRemoveRecords(testCase);
    readWriteAndRemoveRecordsLots(testCase);
    partialRecordRetrieval(testCase);
    bigRecordRetrieval(testCase);
    testIncrementRecord(testCase);
    testSetRecord(testCase);
    testBulkRemoveRecords(testCase);
    testBulkSetRecords(testCase);
    constructDestructAndDelete(testCase);
    test_stKVDatabaseConf_constructFromString_tokyoCabinet(testCase);
    test_stKVDatabaseConf_constructFromString_mysql(testCase);
    USE_CACHE = 0;
}

static void test_cacheWithClearing(CuTest *testCase) {
    /*
     * Tests all the test functions with a cache and no clearing.
     * These tests clear the cache between the adds and the retrieves.
     */
    USE_CACHE = 1;
    CLEAR_CACHE = 1;
    readWriteAndRemoveRecords(testCase);
    readWriteAndRemoveRecordsLots(testCase);
    partialRecordRetrieval(testCase);
    bigRecordRetrieval(testCase);
    testIncrementRecord(testCase);
    testSetRecord(testCase);
    testBulkRemoveRecords(testCase);
    testBulkSetRecords(testCase);
    constructDestructAndDelete(testCase);
    test_stKVDatabaseConf_constructFromString_tokyoCabinet(testCase);
    test_stKVDatabaseConf_constructFromString_mysql(testCase);
    USE_CACHE = 0;
    CLEAR_CACHE = 1;
}

static CuSuite* stKVDatabaseTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, readWriteAndRemoveRecords);
    SUITE_ADD_TEST(suite, readWriteAndUpdateIntRecords);
    SUITE_ADD_TEST(suite, readWriteAndRemoveRecordsLots);
    SUITE_ADD_TEST(suite, partialRecordRetrieval);
    SUITE_ADD_TEST(suite, bigRecordRetrieval);
    SUITE_ADD_TEST(suite, testIncrementRecord);
    SUITE_ADD_TEST(suite, testSetRecord);
    SUITE_ADD_TEST(suite, testBulkRemoveRecords);
    SUITE_ADD_TEST(suite, testBulkSetRecords);
    SUITE_ADD_TEST(suite, constructDestructAndDelete);
    SUITE_ADD_TEST(suite, test_stKVDatabaseConf_constructFromString_tokyoCabinet);
    SUITE_ADD_TEST(suite, test_cache);
    SUITE_ADD_TEST(suite, test_cacheWithClearing);
    SUITE_ADD_TEST(suite, test_stKVDatabaseConf_constructFromString_mysql);
    SUITE_ADD_TEST(suite, test_stKVDatabaseConf_constructFromString_postgresql);
    return suite;
}

static int runTests(void) {
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();
    CuSuiteAddSuite(suite, stKVDatabaseTestSuite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
    return suite->failCount > 0;
}

int main(int argc, char * const *argv) {
    const char *desc = "kvDatabaseTests [options]\n"
        "\n"
        "Run key/value database tests.\n";
    conf = kvDatabaseTestParseOptions(argc, argv, desc, 0, 0, NULL, NULL);
    return runTests();
}

