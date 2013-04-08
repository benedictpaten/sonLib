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

static void test_stCompression_compressAndDecompressP(CuTest *testCase, int64_t rounds, int64_t minSize, int64_t maxSize) {
    /*
     * Exercises the two compression functions.
     */
    for(int64_t i=0; i<rounds; i++) {
        int64_t size = st_randomInt64(minSize, maxSize);
        char *randomString = st_malloc(size);
        for(int64_t j=0; j<size; j++) {
            randomString[j] = (char)st_randomInt(0, 4); //limited alphabet should me we get good compression
        }
        int64_t level = st_randomInt(-1, 10);
        int64_t compressedSizeInBytes;
        //Do the compression
        void *compressedString = stCompression_compress(randomString, sizeof(char)*size, &compressedSizeInBytes, level);
        CuAssertTrue(testCase, compressedSizeInBytes >= 0);
        //Now decompress
        int64_t size2;
        char *randomString2 = stCompression_decompress(compressedString, compressedSizeInBytes, &size2);
        CuAssertTrue(testCase, size == size2);
        for(int64_t j=0; j<size; j++) {
            CuAssertIntEquals(testCase, randomString[j], randomString2[j]);
        }
        st_logDebug("I did a round of compression and uncompression: I got %i bytes to compress and %i bytes compressed\n", size, (int64_t)compressedSizeInBytes);
        free(randomString);
        free(randomString2);
    }
}

/*
 * Does a large number of rounds of compressions and decompression of small strings.
 */
static void test_stCompression_compressAndDecompress_Lots(CuTest *testCase) {
    test_stCompression_compressAndDecompressP(testCase, 1000, 0, 100);
}

/*
 * Does a small number of rounds of large strings.
 */
static void test_stCompression_compressAndDecompress_Big(CuTest *testCase) {
    test_stCompression_compressAndDecompressP(testCase, 5, 5000000000, 6000000000); //5 to 6 gigabytes
}

CuSuite* sonLib_stCompressionTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_stCompression_compressAndDecompress_Lots);
    SUITE_ADD_TEST(suite, test_stCompression_compressAndDecompress_Big);
    return suite;
}
