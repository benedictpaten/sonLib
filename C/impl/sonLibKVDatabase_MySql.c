/*
 * sonLibKVDatabase_MySql.c
 *
 *  Created on: 2010-08-21
 *      Author: Mark Diekhans
 */

//Database functions

#include "sonLibGlobalsInternal.h"
#include "sonLibKVDatabasePrivate.h"
#include "stSafeC.h"

#ifdef HAVE_MYSQL

#include <mysql.h>
#include <mysqld_error.h>

/* mysql client data object, stored in stKVDatabase object */
typedef struct {
    MYSQL *conn;
    char *table;
} MySqlDb;

static MYSQL_RES *queryStart(MySqlDb *dbImpl, const char *query, ...)
#if defined(__GNUC__)
__attribute__((format(printf, 2, 3)))
#endif
;

static void sqlExec(MySqlDb *dbImpl, char *query, ...)
#if defined(__GNUC__)
__attribute__((format(printf, 2, 3)))
#endif
;

/* generate an exception for the current MySQL error */
static void throwMySqlExcept(MySqlDb *dbImpl, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    char *fmtMsg = stSafeCDynFmtv(msg, args);
    va_end(args);
    stExcept *except = stExcept_new(ST_KV_DATABASE_EXCEPTION_ID, "%s: %s (%d)", fmtMsg, mysql_error(dbImpl->conn), mysql_errno(dbImpl->conn));
    stSafeCFree(fmtMsg);
    stThrow(except);
}

/* start an SQL query, formatting arguments into the query */
static MYSQL_RES *queryStartv(MySqlDb *dbImpl, const char *query, va_list args) {
    char *sql = stSafeCDynFmtv(query, args);
    if (mysql_real_query(dbImpl->conn, sql, strlen(sql)) != 0) {
        stSafeCFree(sql);
        throwMySqlExcept(dbImpl, "query failed \"%s\"", sql);
    }
    MYSQL_RES *rs = mysql_use_result(dbImpl->conn);
    if ((rs == NULL) && (mysql_errno(dbImpl->conn) != 0)) {
        stSafeCFree(sql);
        throwMySqlExcept(dbImpl, "query failed \"%s\"", sql);
    }
    stSafeCFree(sql);
    return rs;
}

/* start an SQL query, formatting arguments into the query */
static MYSQL_RES *queryStart(MySqlDb *dbImpl, const char *query, ...) {
    va_list args;
    va_start(args, query);
    MYSQL_RES *rs = queryStartv(dbImpl, query, args);
    va_end(args);
    return rs;
}

static char **queryNext(MySqlDb *dbImpl, MYSQL_RES *rs) {
    if (rs == NULL) {
        return NULL;
    }
    char **row = mysql_fetch_row(rs);
    if (mysql_errno(dbImpl->conn) != 0) {
        throwMySqlExcept(dbImpl, "query fetch failed");
    }
    return row;
}

static char **queryNextRequired(MySqlDb *dbImpl, MYSQL_RES *rs) {
    char **row = queryNext(dbImpl, rs);
    if (row == NULL) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "query did not return required result");
    }
    return row;
}

static size_t *queryLengths(MySqlDb *dbImpl, MYSQL_RES *rs) {
    size_t *lens = mysql_fetch_lengths(rs);
    if (mysql_errno(dbImpl->conn) != 0) {
        throwMySqlExcept(dbImpl, "mysql_fetch_lengths failed");
    }
    return lens;
}

static void queryEnd(MySqlDb *dbImpl, MYSQL_RES *rs) {
    if (rs != NULL) {
        mysql_free_result(rs);
    }
}

/* SQL escape a byte string, result must be freed */
static char *sqlEscape(MySqlDb *dbImpl, const void *data, size_t size) {
    char *escBuf = stSafeCMalloc((2*size)+1);
    mysql_real_escape_string(dbImpl->conn, escBuf, data, size);
    return escBuf;
}

/* immediate execution of a statement that doesn't return results, formatting
 * arguments into query */
static void sqlExec(MySqlDb *dbImpl, char *query, ...) {
    va_list args;
    va_start(args, query);
    MYSQL_RES *rs = queryStartv(dbImpl, query, args);
    va_end(args);
    if (queryNext(dbImpl, rs) != NULL) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "SQL command should not have returned a result: \"%s\"", query);
    }
    queryEnd(dbImpl, rs);
}

/* connect to a database server */
static MySqlDb *connect(stKVDatabaseConf *conf) {
    MySqlDb *dbImpl = stSafeCCalloc(sizeof(MySqlDb));
    if ((dbImpl->conn = mysql_init(NULL)) == NULL) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "mysql_init failed");
    }
    if (mysql_real_connect(dbImpl->conn, stKVDatabaseConf_getHost(conf), stKVDatabaseConf_getUser(conf), stKVDatabaseConf_getPassword(conf), stKVDatabaseConf_getDatabaseName(conf), stKVDatabaseConf_getPort(conf), NULL, 0) == NULL) {
        throwMySqlExcept(dbImpl, "failed to connect to MySql database: %s on %s as user %s",
                         stKVDatabaseConf_getDatabaseName(conf), stKVDatabaseConf_getHost(conf), stKVDatabaseConf_getUser(conf));
    }
    dbImpl->table = stString_copy(stKVDatabaseConf_getTableName(conf));

    // NOTE: commit will not return an error, this does row-level locking on
    // the select done before the update
    sqlExec(dbImpl, "set autocommit = 0;");
    sqlExec(dbImpl, "set session transaction isolation level serializable;");
    return dbImpl;
}

/* create the keyword/value table */
static void createKVTable(MySqlDb *dbImpl) {
    sqlExec(dbImpl, "drop table if exists %s", dbImpl->table);
    sqlExec(dbImpl, "create table %s (id bigint primary key, data blob) type=INNODB;", dbImpl->table);
}

static void destructDB(stKVDatabase *database) {
    MySqlDb *dbImpl = database->dbImpl;
    mysql_close(dbImpl->conn);
    stSafeCFree(dbImpl->table);
    stSafeCFree(dbImpl);
    database->dbImpl = NULL;
}

static void deleteDB(stKVDatabase *database) {
    MySqlDb *dbImpl = database->dbImpl;
    sqlExec(dbImpl, "drop table %s", dbImpl->table);
    destructDB(database);
}

static void insertRecord(stKVDatabase *database, int64_t key, const void *value, int64_t sizeOfRecord) {
    MySqlDb *dbImpl = database->dbImpl;
    char *buf = sqlEscape(dbImpl, value, sizeOfRecord);
    sqlExec(dbImpl, "insert into %s (id, data) values (%lld, \"%s\")", dbImpl->table, (long long)key, buf);
    stSafeCFree(buf);
}

static void updateRecord(stKVDatabase *database, int64_t key,
                         const void *value, int64_t sizeOfRecord) {
    MySqlDb *dbImpl = database->dbImpl;
    char *buf = sqlEscape(dbImpl, value, sizeOfRecord);
    sqlExec(dbImpl, "update %s set data=\"%s\" where id=%lld", dbImpl->table, buf,  (long long)key);
    stSafeCFree(buf);
}

static int64_t numberOfRecords(stKVDatabase *database) {
    MySqlDb *dbImpl = database->dbImpl;
    MYSQL_RES *rs = queryStart(dbImpl, "select count(*) from %s", dbImpl->table);
    char **row = queryNextRequired(dbImpl, rs);
    int64_t numRecords = stSafeStrToInt64(row[0]);
    queryEnd(dbImpl, rs);
    return numRecords;
}

static void *getRecord2(stKVDatabase *database, int64_t key, int64_t *sizeOfRecord) {
    MySqlDb *dbImpl = database->dbImpl;
    MYSQL_RES *rs = queryStart(dbImpl, "select data from %s where id=%lld", dbImpl->table,  (long long)key);
    char **row = queryNext(dbImpl, rs);
    void *data = NULL;
    int64_t readLen = 0;
    if (row != NULL) {
        readLen = queryLengths(dbImpl, rs)[0];
        data = stSafeCCopyMem(row[0], readLen);
    }
    queryEnd(dbImpl, rs);
    if (sizeOfRecord != NULL) {
        *sizeOfRecord = readLen;
    }
    return data;
}

static void *getRecord(stKVDatabase *database, int64_t key) {
    return getRecord2(database, key, NULL);
}

static void *getPartialRecord(stKVDatabase *database, int64_t key, int64_t zeroBasedByteOffset, int64_t sizeInBytes) {
    MySqlDb *dbImpl = database->dbImpl;
    MYSQL_RES *rs = queryStart(dbImpl, "select substring(data, %lld, %lld) from %s where id=%lld", (long long)zeroBasedByteOffset+1, (long long)sizeInBytes, dbImpl->table, (long long)key);
    char **row = queryNext(dbImpl, rs);
    void *data = NULL;
    int64_t readLen = 0;
    if (row != NULL) {
        readLen = queryLengths(dbImpl, rs)[0];
        data = stSafeCCopyMem(row[0], readLen);
    }
    queryEnd(dbImpl, rs);
    if (readLen != sizeInBytes) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "partial read of key %lld, expected %lld bytes got %lld bytes", (long long)key, (long long)sizeInBytes, (long long)readLen);
    }
    return data;
}

static void removeRecord(stKVDatabase *database, int64_t key) {
    MySqlDb *dbImpl = database->dbImpl;
    sqlExec(dbImpl, "delete from %s where id=%lld", dbImpl->table, (long long)key);
    my_ulonglong numRows = mysql_affected_rows(dbImpl->conn);
    if (numRows == 0) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "remove of non-existent key %lld", (long long)key);
    } 
}

static void startTransaction(stKVDatabase *database) {
    MySqlDb *dbImpl = database->dbImpl;
    sqlExec(dbImpl, "start transaction with consistent snapshot;");
}

static void commitTransaction(stKVDatabase *database) {
    MySqlDb *dbImpl = database->dbImpl;
    sqlExec(dbImpl, "commit;");
}

//initialisation function
void stKVDatabase_initialise_MySql(stKVDatabase *database, stKVDatabaseConf *conf, bool create) {
    database->dbImpl = connect(conf);
    database->destruct = destructDB;
    database->delete = deleteDB;
    database->insertRecord = insertRecord;
    database->updateRecord = updateRecord;
    database->numberOfRecords = numberOfRecords;
    database->getRecord = getRecord;
    database->getRecord2 = getRecord2;
    database->getPartialRecord = getPartialRecord;
    database->removeRecord = removeRecord;
    database->startTransaction = startTransaction;
    database->commitTransaction = commitTransaction;
    if (create) {
        createKVTable(database->dbImpl);
    }
}

#endif
