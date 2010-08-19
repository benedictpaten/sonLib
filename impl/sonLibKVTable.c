/*
 * sonLibKVTable.c
 *
 *  Created on: 18-Aug-2010
 *      Author: benedictpaten
 */


#include "sonLibGlobalsInternal.h"

const char *ST_KV_TABLE_EXCEPTION_ID = "ST_KV_TABLE_EXCEPTION";

struct stKVTable {
    char *name;
    void *database;
    void *table;
    bool applyCompression;
    void (*destruct)(void *, void *);
    void (*removeFromDatabase)(void *, void *);
    void (*writeRecord)(void *, void *, bool, int64_t, const void *, int64_t);
    int64_t (*numberOfRecords)(void *, void *);
    void *(*getRecord)(void *, void *, bool, int64_t key);
    void (*removeRecord)(void *, void *, int64_t key);
};

stKVTable *stKVTable_construct(const char *name, stKVDatabase *database, bool applyCompression) {
    stKVTable *table = st_malloc(sizeof(struct stKVTable));
    table->name = stString_copy(name);
    table->database = database;
    table->applyCompression = applyCompression;
    //Get type of database and fill out table methods..
    return table;
}

void stKVTable_destruct(stKVTable *table) {
    table->destruct(table->database, table->table);
    free(table->name);
    free(table);
}

const char *stKVTable_getName(stKVTable *table) {
    return table->name;
}

stKVDatabase *stKVTable_getDatabase(stKVTable *table) {
    return table->database;
}

void stKVTable_removeFromDatabase(stKVTable *table) {
    table->removeFromDatabase(table->database, table->table);
}

void stKVTable_writeRecord(stKVTable *table, int64_t key, const void *value, int64_t sizeOfRecord) {
    table->writeRecord(table->database, table->table, table->applyCompression, key, value, sizeOfRecord);
}

int64_t stKVTable_getNumberOfRecords(stKVTable *table) {
    return table->numberOfRecords(table->database, table->table);
}

void *stKVTable_getRecord(stKVTable *table, int64_t key) {
    return table->getRecord(table->database, table->table, table->applyCompression, key);
}

void stKVTable_removeRecord(stKVTable *table, int64_t key) {
    table->removeRecord(table->database, table->table, key);
}
