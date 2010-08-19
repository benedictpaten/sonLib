/*
 * sonLibKVDatabase_TokyoCabinet.c
 *
 *  Created on: 18-Aug-2010
 *      Author: benedictpaten
 */

//Database functions

#include "sonLibGlobalsInternal.h"
#include "sonLibKVDatabasePrivate.h"

static void destructDB(void *database) {

}

static void deleteDB(void *database) {

}

static int64_t numberOfTables(void *database) {
    return 0;
}

static stKVTable *getTable(void *database, const char *name) {
    return 0;
}

static void startTransaction(void *database) {

}

static void commitTransaction(void *database) {

}

void stKVDatabase_initialise_tokyoCabinet(stKVDatabase *database) {
    database->destruct = (void (*)(void *))destructDB;
    database->delete = (void (*)(void *))deleteDB;
    database->numberOfTables = (int64_t (*)(void *))numberOfTables;
    database->getTable = (stKVTable *(*)(void *, const char *))getTable;
    database->startTransaction = (void (*)(void *))startTransaction;
    database->commitTransaction = (void (*)(void *))commitTransaction;
}

//Table functions

static void destructTable(void *database, void *table) {
}

static void removeFromDatabase(void *database, void *table) {

}

static void writeRecord(void *database, void *table, bool applyCompression, int64_t key, const void *value, int64_t sizeOfRecord) {

}

static int64_t numberOfRecords(void *database, void *table) {
    return 0;
}

static void *getRecord(void *database, void *table, bool applyCompression, int64_t key) {
    return 0;
}

static void removeRecord(void *database, void *table, int64_t key) {

}

void stKVTable_initialise_tokyoCabinet(stKVTable *table) {
    table->destruct = (void (*)(void *, void *))destructTable;
    table->removeFromDatabase = (void (*)(void *, void *))removeFromDatabase;
    table->writeRecord = (void (*)(void *, void *, bool, int64_t, const void *, int64_t))writeRecord;
    table->numberOfRecords = (int64_t (*)(void *, void *))numberOfRecords;
    table->getRecord = (void *(*)(void *, void *, bool, int64_t))getRecord;
    table->removeRecord = (void (*)(void *, void *, int64_t))removeRecord;
}
