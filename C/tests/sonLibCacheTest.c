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

static stCache *cache = NULL;
static int64_t recordSize;

static void teardown() {
    if(cache != NULL) {
        stCache_destruct(cache);
        cache = NULL;
    }
}

static void setup() {
    teardown();
    cache = stCache_construct();
}

static void readAndUpdateRecord(CuTest *testCase) {
    setup();

    CuAssertTrue(testCase, stCache_getRecord(cache, 1, 0, INT64_MAX, &recordSize) == NULL);
    CuAssertTrue(testCase, !stCache_containsRecord(cache, 1, 0, INT64_MAX));

    stCache_setRecord(cache, 1, 5, 6, "hello");
    stCache_setRecord(cache, 1, 12, 6, "world");

    CuAssertTrue(testCase, stCache_getRecord(cache, 1, 0, 5, &recordSize) == NULL); //Check prefix returns false
    CuAssertTrue(testCase, !stCache_containsRecord(cache, 1, 0, 5));

    CuAssertTrue(testCase, stCache_getRecord(cache, 1, 4, 2, &recordSize) == NULL); //Check prefix overlap returns false
    CuAssertTrue(testCase, !stCache_containsRecord(cache, 1, 4, 2));

    CuAssertStrEquals(testCase, "hello", stCache_getRecord(cache, 1, 5, 6, &recordSize)); //Check we can get the first word
    CuAssertTrue(testCase, recordSize == 6);
    CuAssertTrue(testCase, stCache_containsRecord(cache, 1, 5, 6));

    CuAssertTrue(testCase, stCache_getRecord(cache, 1, 5, 7, &recordSize) == NULL); //Check suffix overlap returns false
    CuAssertTrue(testCase, !stCache_containsRecord(cache, 1, 5, 7));

    CuAssertTrue(testCase, stCache_getRecord(cache, 1, 11, 1, &recordSize) == NULL); //Check in between returns false
    CuAssertTrue(testCase, !stCache_containsRecord(cache, 1, 11, 1));

    CuAssertTrue(testCase, stCache_getRecord(cache, 1, 11, 2, &recordSize) == NULL); //Check prefix of world returns false
    CuAssertTrue(testCase, !stCache_containsRecord(cache, 1, 11, 2));

    CuAssertStrEquals(testCase, "world", stCache_getRecord(cache, 1, 12, 6, &recordSize)); //Get the second word
    CuAssertTrue(testCase, recordSize == 6);
    CuAssertTrue(testCase, stCache_containsRecord(cache, 1, 12, 6));

    CuAssertStrEquals(testCase, "", stCache_getRecord(cache, 1, 17, 1, &recordSize)); //Get part of the second word
    CuAssertTrue(testCase, recordSize == 1);
    CuAssertTrue(testCase, stCache_containsRecord(cache, 1, 17, 1));

    CuAssertTrue(testCase, stCache_getRecord(cache, 1, 17, 2, &recordSize) == NULL); //Check suffix overlap returns false
    CuAssertTrue(testCase, !stCache_containsRecord(cache, 1, 17, 2));

    CuAssertTrue(testCase, stCache_getRecord(cache, 1, 5, 13, &recordSize) == NULL); //Check suffix overlap returns false
    CuAssertTrue(testCase, !stCache_containsRecord(cache, 1, 5, 13));

    stCache_setRecord(cache, 1, 10, 2, "  ");

    CuAssertStrEquals(testCase, "hello  world", stCache_getRecord(cache, 1, 5, INT64_MAX, &recordSize)); //Check we can get the first word
    CuAssertTrue(testCase, recordSize == 13);
    CuAssertTrue(testCase, stCache_containsRecord(cache, 1, 5, 13));

    stCache_setRecord(cache, 1, 5, 6, "see ya");

    CuAssertStrEquals(testCase, "see ya world", stCache_getRecord(cache, 1, 5, 13, &recordSize)); //Check we can get the first word
    CuAssertTrue(testCase, recordSize == 13);
    CuAssertTrue(testCase, stCache_containsRecord(cache, 1, 5, 13));
    CuAssertTrue(testCase, stCache_containsRecord(cache, 1, 5, INT64_MAX));

    stCache_clear(cache);

    CuAssertTrue(testCase, !stCache_containsRecord(cache, 1, 12, 6)); //Check its gone

    teardown();
}

static void readAndUpdateRecords(CuTest *testCase) {
    setup();

    stCache_setRecord(cache, 1, 0, 6, "hello");
    stCache_setRecord(cache, 1, 0, 6, "world");
    stCache_setRecord(cache, INT64_MAX-1, 0, 8, "goodbye");
    stCache_setRecord(cache, 3, 0, 6, "cruel");
    stCache_setRecord(cache, INT32_MIN, 0, 6, "earth");

    CuAssertStrEquals(testCase, "world", stCache_getRecord(cache, 1, 0, INT64_MAX, &recordSize)); //Get the second word
    CuAssertTrue(testCase, recordSize == 6);
    CuAssertTrue(testCase, stCache_containsRecord(cache, 1, 0, INT64_MAX));

    CuAssertStrEquals(testCase, "goodbye", stCache_getRecord(cache, INT64_MAX-1, 0, INT64_MAX, &recordSize)); //Get the second word
    CuAssertTrue(testCase, recordSize == 8);
    CuAssertTrue(testCase, stCache_containsRecord(cache, INT64_MAX-1, 0, INT64_MAX));

    CuAssertStrEquals(testCase, "cruel", stCache_getRecord(cache, 3, 0, INT64_MAX, &recordSize)); //Get the second word
    CuAssertTrue(testCase, recordSize == 6);
    CuAssertTrue(testCase, stCache_containsRecord(cache, 3, 0, INT64_MAX));

    CuAssertStrEquals(testCase, "earth", stCache_getRecord(cache, INT32_MIN, 0, INT64_MAX, &recordSize)); //Get the second word
    CuAssertTrue(testCase, recordSize == 6);
    CuAssertTrue(testCase, stCache_containsRecord(cache, INT32_MIN, 0, INT64_MAX));

    teardown();
}

CuSuite* stCacheSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, readAndUpdateRecord);
    SUITE_ADD_TEST(suite, readAndUpdateRecords);

    return suite;
}


