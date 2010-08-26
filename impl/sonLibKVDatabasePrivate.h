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
    bool applyCompression;
    bool transactionStarted;
    bool deleted;
    void (*destruct)(void *);
    void (*delete)(void *, const char *);
    void (*writeRecord)(void *, bool, int64_t, const void *, int64_t);
    int64_t (*numberOfRecords)(void *);
    void *(*getRecord)(void *, bool, int64_t key);
    void (*removeRecord)(void *, int64_t key);
    void (*startTransaction)(void *);
    void (*commitTransaction)(void *);
};

/*
 * Function initialises the pointers of the stKVDatabase object with functions for tokyoCabinet.
 */
void stKVDatabase_initialise_tokyoCabinet(stKVDatabase *database);


#endif /* SONLIBKVDATABASEPRIVATE_H_ */
