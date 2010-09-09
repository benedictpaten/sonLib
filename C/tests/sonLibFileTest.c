#include "sonLibGlobalsTest.h"

static char *tempFileName = "sonLibFileTestTempFile.txt";

static void testSetup() {
    FILE *fileHandle = fopen(tempFileName, "w");
    fprintf(fileHandle, "hello world\n");
    fprintf(fileHandle, "foo bar 123456\n");
    fprintf(fileHandle, " \n");
    fprintf(fileHandle, "\n");
    fprintf(fileHandle, "bye bye\n");
    fprintf(fileHandle, "\t");
    fclose(fileHandle);
}

static void testTeardown() {
    st_system("rm -f %s", tempFileName);
}

static void testFile_getLineFromFile(CuTest *testCase) {
    testSetup();
    FILE *fileHandle = fopen(tempFileName, "r");
    CuAssertStrEquals(testCase, "hello world", stFile_getLineFromFile(fileHandle));
    CuAssertStrEquals(testCase, "foo bar 123456", stFile_getLineFromFile(fileHandle));
    CuAssertStrEquals(testCase, " ", stFile_getLineFromFile(fileHandle));
    CuAssertStrEquals(testCase, "", stFile_getLineFromFile(fileHandle));
    CuAssertStrEquals(testCase, "bye bye", stFile_getLineFromFile(fileHandle));
    CuAssertStrEquals(testCase, "\t", stFile_getLineFromFile(fileHandle));
    CuAssertTrue(testCase, NULL == stFile_getLineFromFile(fileHandle));
    CuAssertTrue(testCase, NULL == stFile_getLineFromFile(fileHandle));
    CuAssertTrue(testCase, NULL == stFile_getLineFromFile(fileHandle));
    CuAssertTrue(testCase, NULL == stFile_getLineFromFile(fileHandle));
    CuAssertTrue(testCase, NULL == stFile_getLineFromFile(fileHandle));
    fclose(fileHandle);
    testTeardown();
}

CuSuite* sonLibFileTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testFile_getLineFromFile);

    return suite;
}
