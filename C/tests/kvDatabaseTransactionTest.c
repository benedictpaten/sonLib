/*
 * Manually run test to check reporting update collision.
 */

#include "sonLibGlobalsTest.h"
#include "kvDatabaseTestCommon.h"

static int64_t testRec1Id = 1;
static const char *testRec1Data0 = "Nobody talks to a horse, of course";
static const char *testRec1Data1 = "However, some talk to their computer";
static const char *testRec1Data2 = "and some talk to their cat";

static void initDatabase(stKVDatabaseConf *conf) {
    // start fresh
    stKVDatabase *database = stKVDatabase_construct(conf, true);
    stKVDatabase_deleteFromDisk(database);
    stKVDatabase_destruct(database);

    // create with one record
    database = stKVDatabase_construct(conf, true);
    stKVDatabase_startTransaction(database);
    stKVDatabase_insertRecord(database, testRec1Id, testRec1Data0, strlen(testRec1Data0)+1);
    stKVDatabase_commitTransaction(database);
    stKVDatabase_destruct(database);
}

static void prompt(const char *msg) {
    fflush(stdout);
    fflush(stderr);
    printf("%s (press return to continue): ", msg);
    fflush(stdout);
    getc(stdin);
}

static void runTest(stKVDatabaseConf *conf, bool readBeforeUpdate, const char *data) {
    stKVDatabase *database = stKVDatabase_construct(conf, false);
    stKVDatabase_startTransaction(database);
    prompt("started transaction");
    if (readBeforeUpdate) {
        prompt("ready to read record");
        char *inData = stKVDatabase_getRecord(database, testRec1Id);
        free(inData);
    }
    prompt("ready to update record");
    stKVDatabase_updateRecord(database, testRec1Id, data, strlen(data)+1);
    prompt("ready to commit");
    stKVDatabase_commitTransaction(database);
    stKVDatabase_destruct(database);
}

int main(int argc, char *const *argv) {
    const char *desc = "kvDatabaseTests [options] action\n"
        "\n"
        "Run test of transaction collisions\n"
        "The idea is to run two instances of this program\n"
        "in different windows. Controlling how they handle collisions\n"
        "my when return is pressed to allow the program to go to the\n"
        "next step.\n"
        "\n"
        "The action argument is one of:\n"
        "  init - initialize the database\n"
        "  update1 - update process 1, without reading\n"
        "  update2- process 2, without reading\n"
        "  readUpdate1 - read, pause update process 1\n"
        "  readUpdate2 - read, pause update process 2\n";

    char *positional[1];
    stKVDatabaseConf *conf = kvDatabaseTestParseOptions(argc, argv, desc, 1, 1, positional, NULL);
    if (stString_eq(positional[0], "init")) {
        initDatabase(conf);
    } else if (stString_eq(positional[0], "update1")) {
        runTest(conf, false, testRec1Data1);
    } else if (stString_eq(positional[0], "update2")) {
        runTest(conf, false, testRec1Data2);
    } else if (stString_eq(positional[0], "readUpdate1")) {
        runTest(conf, true, testRec1Data1);
    } else if (stString_eq(positional[0], "readUpdate2")) {
        runTest(conf, true, testRec1Data2);
    } else {
        st_errAbort("invalid argument value \"%s\"\n%s", positional[0], desc);
    }
    return 0;
}

