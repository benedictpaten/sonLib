/*
 * Copyright (C) 2006-2012 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibRandomTest.c
 *
 *  Created on: 22-Jun-2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsTest.h"

static void test_stCompression_compressAndDecompressP(CuTest *testCase, int32_t rounds, int64_t minSize, int64_t maxSize) {
    /*
     * Exercises the two compression functions.
     */
    for(int32_t i=0; i<rounds; i++) {
        int64_t size = st_randomInt64(minSize, maxSize);
        char *randomString = st_malloc(size);
        for(int32_t j=0; j<size; j++) {
            randomString[j] = (char)st_randomInt(0, 100); //nearly random string
        }
        int32_t level = st_randomInt(-1, 10);
        int64_t compressedSizeInBytes;
        //Do the compression
        void *compressedString = stCompression_compress(randomString, sizeof(char)*size, &compressedSizeInBytes, level);
        CuAssertTrue(testCase, compressedSizeInBytes >= 0);
        st_logDebug("I did a round of compression: I got %" PRIi64  " bytes to compress and %" PRIi64 " bytes compressed\n", size, compressedSizeInBytes);
        //Now decompress
        int64_t size2;
        char *randomString2 = stCompression_decompress(compressedString, compressedSizeInBytes, &size2);
        CuAssertTrue(testCase, size == size2);
        for(int32_t j=0; j<size; j++) {
            CuAssertIntEquals(testCase, randomString[j], randomString2[j]);
        }
        free(randomString);
        free(randomString2);
    }
}

/*
 * Does a large number of rounds of compressions and decompression of small strings.
 */
static void test_stCompression_compressAndDecompress_Lots(CuTest *testCase) {
    test_stCompression_compressAndDecompressP(testCase, 100, 0, 500000);
}

/*
 * Does a small number of rounds of large strings.
 */
static void test_stCompression_compressAndDecompress_Big(CuTest *testCase) {
    test_stCompression_compressAndDecompressP(testCase, 5, 10000000, 50000000);
}

CuSuite* sonLib_stCompressionTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_stCompression_compressAndDecompress_Lots);
    SUITE_ADD_TEST(suite, test_stCompression_compressAndDecompress_Big);
    return suite;
}
