/*
 * sonLibKVDatabaseTest.c
 *
 */

#include "sonLibGlobalsTest.h"

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

static stKVDatabaseConf *getConf() {
    static stKVDatabaseConf *conf = NULL;
    if (conf == NULL) {
#if 1
        conf = stKVDatabaseConf_constructTokyoCabinet("testTCDatabase");
        fprintf(stderr, "running Tokyo Cabinet sonLibKVDatabase tests\n");
#elif 0
        //host="localhost" port="0" user="root" password="" database_name="cactus"
        conf = stKVDatabaseConf_constructMySql("localhost", 0, "root", "", "cactus", "cactusDbTest");
        fprintf(stderr, "running MySQL local sonLibKVDatabase tests\n");
#else
        conf = stKVDatabaseConf_constructMySql("kolossus-10", 0, "cactus", "cactus", "cactus", "cactusDbTest");
        fprintf(stderr, "running MySQL kolossus-10 sonLibKVDatabase tests\n");
#endif
    }
    return conf;
}

static void setup() {
    teardown();
    database = stKVDatabase_construct(getConf(), true);
    teardown();
    database = stKVDatabase_construct(getConf(), true);
    if(USE_CACHE) {
        stKVDatabase_makeMemCache(database, 50000, 5); //Makes a cache with a 50k cache.
    }
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

    stKVDatabase_commitTransaction(database);
    teardown();
}

static void partialRecordRetrieval(CuTest *testCase) {
    setup();
    stKVDatabase_startTransaction(database);

    //Make some number of large records
    stList *records = stList_construct3(0, free);
    stList *recordSizes = stList_construct3(0,
            (void(*)(void *)) stIntTuple_destruct);
    for (int32_t i = 0; i < 300; i++) {
        int32_t size = st_randomInt(0, 80);
        size = size *size *size; //Use cubic size distribution
        char *randomRecord = st_malloc(size*sizeof(char));
        for(int32_t j=0; j<size; j++) {
            randomRecord[j] = (char)st_randomInt(0, 100);
        }
        stList_append(records, randomRecord);
        stList_append(recordSizes, stIntTuple_construct(1, size));
        CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, i));
        stKVDatabase_insertRecord(database, i, randomRecord, size*sizeof(char));
        CuAssertTrue(testCase, stKVDatabase_containsRecord(database, i));
        //st_uglyf("I am creating the record %i %i\n", i, size);
    }
    stKVDatabase_commitTransaction(database);

    if(CLEAR_CACHE) {
        stKVDatabase_clearCache(database);
    }

    while (st_random() > 0.001) {
        int32_t recordKey = st_randomInt(0, stList_length(records));
        CuAssertTrue(testCase, stKVDatabase_containsRecord(database, recordKey));

        char *record = stList_get(records, recordKey);
        int32_t size = stIntTuple_getPosition(
                stList_get(recordSizes, recordKey), 0);

        //Get partial record
        int32_t start = size > 0 ? st_randomInt(0, size) : 0;
        int32_t partialSize = size - start > 0 ? st_randomInt(start, size) - start : 0;
        assert(start >= 0);
        assert(partialSize >= 0);
        assert(partialSize + start <= size);
        //st_uglyf("I am getting record %i %i %i %i\n", recordKey, start, partialSize, size);
        char *partialRecord = stKVDatabase_getPartialRecord(database,
                recordKey, start*sizeof(char), partialSize*sizeof(char), size*sizeof(char));

        //Check they are equivalent..
        for (int32_t i = 0; i < partialSize; i++) {
            if(record[start + i] != partialRecord[i]) {
                st_uglyf("There was a difference %i %i for record %i %i\n", record[start + i], partialRecord[i], i, partialSize);
            }
            //CuAssertTrue(testCase, record[start + i] == partialRecord[i]);
        }

        //Check we can not get out of bounds.. (start less than zero)
        stTry {
            stKVDatabase_getPartialRecord(database, recordKey, -1, 1, size*sizeof(char));
        }
        stCatch(except) {
            CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
        }stTryEnd

        //Check we can not get out of bounds.. (start greater than index start)
        stTry {
            stKVDatabase_getPartialRecord(database, recordKey, size, 1, size*sizeof(char));
        }
        stCatch(except) {
            CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
        }stTryEnd

        //Check we can not get out of bounds.. (total size if greater than record length)
        stTry {
            stKVDatabase_getPartialRecord(database, recordKey, 0, size+1, size*sizeof(char));
        }
        stCatch(except) {
            CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
        }stTryEnd

        //Check we can not get non existent record
        stTry {
            stKVDatabase_getPartialRecord(database, 1000000, 0, size, size*sizeof(char));
        }
        stCatch(except) {
            CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
        }stTryEnd
    }

    stList_destruct(records);
    stList_destruct(recordSizes);

    teardown();
}

static void testTransactions(CuTest *testCase) {
    /*
     * Tests starting and committing a transaction, does not attempt an abort.
     */
    setup();
    //The setup method starts a transaction to allow you to create tables.
    //We will try committing the transaction and then reclaiming the created

    //First create some stuff to store..
    stKVDatabase_startTransaction(database); //need to start a transaction to create tables etc.
    //Try starting the transaction again to capture exception
    stTry {
            stKVDatabase_startTransaction(database);
            CuAssertTrue(testCase, 0);
        }
        stCatch(except)
                {
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
        }
        stCatch(except)
                {
                    CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
                }stTryEnd;

    stKVDatabase_destruct(database);

    //Now re-open the database and check it is as expected:
    database = stKVDatabase_construct(getConf(), false);
    CuAssertIntEquals(testCase, 3, stKVDatabase_getNumberOfRecords(database));
    CuAssertStrEquals(testCase, "Red", stKVDatabase_getRecord(database, 1));
    CuAssertStrEquals(testCase, "Green", stKVDatabase_getRecord(database, 2));
    CuAssertStrEquals(testCase, "Black", stKVDatabase_getRecord(database, 0));

    teardown();
}

static void testAbortedTransaction(CuTest *testCase) {
    /*
     * Tests aborting transactions.
     */
    setup();

    stTry { //Test we can not abort a transaction that has not been started
        stKVDatabase_abortTransaction(database);
        CuAssertTrue(testCase, 0);
    }
    stCatch(except)
    {
        CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
    }stTryEnd

    //First create some stuff to store..
    stKVDatabase_startTransaction(database); //need to start a transaction to create tables etc.
    //Check records not in
    CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, 0));
    CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, 1));
    CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, 2));
    //Add records
    stKVDatabase_insertRecord(database, 1, "Red", sizeof(char) * 4);
    stKVDatabase_insertRecord(database, 2, "Green", sizeof(char) * 6);
    stKVDatabase_insertRecord(database, 0, "Black", sizeof(char) * 5);
    //Check they are in
    CuAssertTrue(testCase, stKVDatabase_containsRecord(database, 0));
    CuAssertTrue(testCase, stKVDatabase_containsRecord(database, 1));
    CuAssertTrue(testCase, stKVDatabase_containsRecord(database, 2));

    stKVDatabase_abortTransaction(database); //Now we abort the transaction..

    //Check the records are no longer in the database
    CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, 0));
    CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, 1));
    CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, 2));

    stTry { //Test we can not abort a transaction twice.
        stKVDatabase_abortTransaction(database);
        CuAssertTrue(testCase, 0);
    }
    stCatch(except)
    {
        CuAssertTrue(testCase, stExcept_getId(except) == ST_KV_DATABASE_EXCEPTION_ID);
    }stTryEnd

    //Now we try again..
    stKVDatabase_startTransaction(database);
    //And we should be able to insert these records as the previous transaction was aborted
    stKVDatabase_insertRecord(database, 1, "Red", sizeof(char) * 4);
    stKVDatabase_insertRecord(database, 2, "Green", sizeof(char) * 6);
    stKVDatabase_insertRecord(database, 0, "Black", sizeof(char) * 5);
    stKVDatabase_commitTransaction(database); //Now we completed things just fine!

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
        stKVDatabase_startTransaction(database);
        //st_uglyf("I am inserting record %i %i\n", i, size);
        stKVDatabase_insertRecord(database, i, randomRecord, size
                * sizeof(char));
        stKVDatabase_commitTransaction(database);

        if(CLEAR_CACHE) {
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
static void readWriteAndRemoveRecordsLotsCheck(CuTest *testCase,
        stSortedSet *set, int valueMult) {
    CuAssertIntEquals(testCase, stSortedSet_size(set), stKVDatabase_getNumberOfRecords(database));
    stSortedSetIterator *it = stSortedSet_getIterator(set);
    stIntTuple *tuple;
    while ((tuple = stSortedSet_getNext(it)) != NULL) {
        int32_t *value = (int32_t *) stKVDatabase_getRecord(database,
                stIntTuple_getPosition(tuple, 0));
        CuAssertTrue(testCase, stKVDatabase_containsRecord(database, stIntTuple_getPosition(tuple, 0)));
        CuAssertIntEquals(testCase, valueMult*stIntTuple_getPosition(tuple, 0), *value);
        free(value);
    }
    stSortedSet_destructIterator(it);
}

static void readWriteAndRemoveRecordsLotsIteration(CuTest *testCase,
        int numRecords, bool reopenDatabase) {
    stKVDatabase_startTransaction(database);

    //Make a big old list of records..
    stSortedSet *set = stSortedSet_construct3((int(*)(const void *,
            const void *)) stIntTuple_cmpFn,
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

    if(CLEAR_CACHE) {
        stKVDatabase_clearCache(database);
    }

    //Update all records to negate values
    stSortedSetIterator *it = stSortedSet_getIterator(set);
    stIntTuple *tuple;
    while ((tuple = stSortedSet_getNext(it)) != NULL) {
        int32_t *value = (int32_t *) stKVDatabase_getRecord(database,
                stIntTuple_getPosition(tuple, 0));
        *value *= -1;
        stKVDatabase_updateRecord(database, stIntTuple_getPosition(tuple, 0),
                value, sizeof(int32_t));
        CuAssertTrue(testCase, stKVDatabase_containsRecord(database, stIntTuple_getPosition(tuple, 0)));
        free(value);
    }
    stSortedSet_destructIterator(it);

    readWriteAndRemoveRecordsLotsCheck(testCase, set, -1);

    //Try optionally committing the transaction and reloading the database..
    if (reopenDatabase) {
        stKVDatabase_commitTransaction(database);
        stKVDatabase_destruct(database);
        database = stKVDatabase_construct(getConf(), false);
        stKVDatabase_startTransaction(database);
    }

    //Now remove each one..
    it = stSortedSet_getIterator(set);
    while ((tuple = stSortedSet_getNext(it)) != NULL) {
        CuAssertTrue(testCase, stKVDatabase_containsRecord(database, stIntTuple_getPosition(tuple, 0)));
        stKVDatabase_removeRecord(database, stIntTuple_getPosition(tuple, 0));
        CuAssertTrue(testCase, !stKVDatabase_containsRecord(database, stIntTuple_getPosition(tuple, 0)));
        //Test we get exception if we remove twice.
        stTry {
                stKVDatabase_removeRecord(database, stIntTuple_getPosition(
                        tuple, 0));
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
    stKVDatabase_commitTransaction(database);
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

static void test_stKVDatabaseConf_constructFromString_tokyoCabinet(
        CuTest *testCase) {
    const char
            *tokyoCabinetTestString =
                    "<st_kv_database_conf type='tokyo_cabinet'><tokyo_cabinet database_dir='foo'/></st_kv_database_conf>";
    stKVDatabaseConf *conf = stKVDatabaseConf_constructFromString(
            tokyoCabinetTestString);
    CuAssertTrue(testCase, stKVDatabaseConf_getType(conf) == stKVDatabaseTypeTokyoCabinet);
    CuAssertStrEquals(testCase, "foo", stKVDatabaseConf_getDir(conf));
}

static void test_stKVDatabaseConf_constructFromString_mysql(CuTest *testCase) {
#ifndef HAVE_MYSQL
    return;
#endif
    const char
            *tokyoCabinetTestString =
                    "<st_kv_database_conf type='mysql'><mysql host='enormous' port='5' user='foo' password='bar' database_name='mammals' table_name='flowers'/></st_kv_database_conf>";
    stKVDatabaseConf *conf = stKVDatabaseConf_constructFromString(
            tokyoCabinetTestString);
    CuAssertTrue(testCase, stKVDatabaseConf_getType(conf) == stKVDatabaseTypeMySql);
    CuAssertTrue(testCase, stKVDatabaseConf_getDir(conf) == NULL);
    CuAssertStrEquals(testCase, "enormous", stKVDatabaseConf_getHost(conf));
    CuAssertIntEquals(testCase, 5, stKVDatabaseConf_getPort(conf));
    CuAssertStrEquals(testCase, "foo", stKVDatabaseConf_getUser(conf));
    CuAssertStrEquals(testCase, "bar", stKVDatabaseConf_getPassword(conf));
    CuAssertStrEquals(testCase, "mammals", stKVDatabaseConf_getDatabaseName(conf));
    CuAssertStrEquals(testCase, "flowers", stKVDatabaseConf_getTableName(conf));
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
    testTransactions(testCase);
    testAbortedTransaction(testCase);
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
    testTransactions(testCase);
    testAbortedTransaction(testCase);
    constructDestructAndDelete(testCase);
    test_stKVDatabaseConf_constructFromString_tokyoCabinet(testCase);
    test_stKVDatabaseConf_constructFromString_mysql(testCase);
    USE_CACHE = 0;
    CLEAR_CACHE = 1;
}

CuSuite* sonLib_stKVDatabaseTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, readWriteAndRemoveRecords);
    SUITE_ADD_TEST(suite, readWriteAndRemoveRecordsLots);
    SUITE_ADD_TEST(suite, partialRecordRetrieval);
    SUITE_ADD_TEST(suite, bigRecordRetrieval);
    SUITE_ADD_TEST(suite, testTransactions);
    SUITE_ADD_TEST(suite, testAbortedTransaction);
    SUITE_ADD_TEST(suite, constructDestructAndDelete);
    SUITE_ADD_TEST(suite, test_stKVDatabaseConf_constructFromString_tokyoCabinet);
    SUITE_ADD_TEST(suite, test_cache);
    SUITE_ADD_TEST(suite, test_cacheWithClearing);
    SUITE_ADD_TEST(suite, test_stKVDatabaseConf_constructFromString_mysql);
    return suite;
}

