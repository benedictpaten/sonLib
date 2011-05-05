/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibKVDatabasePrivate.h
 *
 *  Created on: 18 Aug 2010
 *      Author: benedictpaten
 */

#ifndef SONLIBKVDATABASEPRIVATE_H_
#define SONLIBKVDATABASEPRIVATE_H_

struct stKVDatabase {
    stKVDatabaseConf *conf;
    void *dbImpl;
    void *cache;
    bool transactionStarted;
    bool deleted;
    void (*destruct)(stKVDatabase *);
    void (*deleteDatabase)(stKVDatabase *);
    bool (*containsRecord)(stKVDatabase *, int64_t);
    void (*insertRecord)(stKVDatabase *, int64_t, const void *, int64_t);
    void (*updateRecord)(stKVDatabase *, int64_t, const void *, int64_t);
    int64_t (*numberOfRecords)(stKVDatabase *);
    void *(*getRecord)(stKVDatabase *, int64_t key);
    void *(*getRecord2)(stKVDatabase *database, int64_t key, int64_t *recordSize);
    void *(*getPartialRecord)(stKVDatabase *database, int64_t key, int64_t zeroBasedByteOffset, int64_t sizeInBytes, int64_t recordSize);
    void (*removeRecord)(stKVDatabase *, int64_t key);
    void (*startTransaction)(stKVDatabase *);
    void (*commitTransaction)(stKVDatabase *);
    void (*abortTransaction)(stKVDatabase *);
    void (*clearCache)(stKVDatabase *);
};

/*
 * Function initialises the pointers of the stKVDatabase object with functions for tokyoCabinet.
 */
void stKVDatabase_initialise_tokyoCabinet(stKVDatabase *database, stKVDatabaseConf *conf, bool create);

/*
 * Function initialises the pointers of the stKVDatabase object with functions for tokyoCabinet.
 */
void stKVDatabase_initialise_tokyoTyrant(stKVDatabase *database, stKVDatabaseConf *conf, bool create);

/*
 * Function initialises the pointers of the stKVDatabase object with functions for MySql.
 */
void stKVDatabase_initialise_MySql(stKVDatabase *database, stKVDatabaseConf *conf, bool create);


/*
 * Function initialises the pointers of the stKVDatabase object with functions PostgreSql .
 */
void stKVDatabase_initialise_PostgreSql(stKVDatabase *database, stKVDatabaseConf *conf, bool create);

#ifdef __cplusplus
extern "C" {
#endif
void stKVDatabase_initialise_kyotoTycoon(stKVDatabase *database, stKVDatabaseConf *conf, bool create);
#ifdef __cplusplus
}
#endif

#endif /* SONLIBKVDATABASEPRIVATE_H_ */
