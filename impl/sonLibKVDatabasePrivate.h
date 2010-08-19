/*
 * sonLibKVDatabasePrivate.h
 *
 *  Created on: 18 Aug 2010
 *      Author: benedictpaten
 */

#ifndef SONLIBKVDATABASEPRIVATE_H_
#define SONLIBKVDATABASEPRIVATE_H_

struct stKVDatabase {
    char *url;
    void *database;
    void (*destruct)(void *);
    void (*delete)(void *);
    int64_t (*numberOfTables)(void *);
    stKVTable *(*getTable)(void *, const char *name);
    void (*startTransaction)(void *);
    void (*commitTransaction)(void *);
};

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

/*
 * Function initialises the pointers of the stKVDatabase object with functions for tokyoCabinet.
 */
void stKVDatabase_initialise_tokyoCabinet(stKVDatabase *database);

/*
 * Function initialises the pointers of the stKVTable object with functions for tokyoCabinet.
 */
void stKVTable_initialise_tokyoCabinet(stKVTable *table);


#endif /* SONLIBKVDATABASEPRIVATE_H_ */
