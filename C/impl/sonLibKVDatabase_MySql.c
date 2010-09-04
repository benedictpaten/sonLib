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

/* is this a mysql error were the transaction should be retried */
static bool isMysqlRetryError(int mysqlErrNo) {
    return (mysqlErrNo == ER_LOCK_DEADLOCK) || (mysqlErrNo == ER_LOCK_WAIT_TIMEOUT);
}

/* create an exception for the current MySQL error */
static stExcept *createMySqlExceptv(MySqlDb *dbImpl, const char *msg, va_list args) {
    char *fmtMsg = stSafeCDynFmtv(msg, args);
    const char *exId = isMysqlRetryError(mysql_errno(dbImpl->conn)) ?  ST_KV_DATABASE_RETRY_TRANSACTION_EXCEPTION_ID : ST_KV_DATABASE_EXCEPTION_ID;
    stExcept *except = stExcept_new(exId, "%s: %s (%d)", fmtMsg, mysql_error(dbImpl->conn), mysql_errno(dbImpl->conn));
    stSafeCFree(fmtMsg);
    return except;
}

/* create an exception for the current MySQL error */
static stExcept *createMySqlExcept(MySqlDb *dbImpl, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    stExcept *except = createMySqlExceptv(dbImpl, msg, args);
    va_end(args);
    return except;
}

/* generate an exception for the current MySQL error */
static void throwMySqlExcept(MySqlDb *dbImpl, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    stExcept *except = createMySqlExceptv(dbImpl, msg, args);
    va_end(args);
    stThrow(except);
}

/* start an SQL query, formatting arguments into the query */
static MYSQL_RES *queryStartv(MySqlDb *dbImpl, const char *query, va_list args) {
    char *sql = stSafeCDynFmtv(query, args);
    if (mysql_real_query(dbImpl->conn, sql, strlen(sql)) != 0) {
        stExcept *ex = createMySqlExcept(dbImpl, "query failed \"%0.60s\"", sql);
        stSafeCFree(sql);
        stThrow(ex);
    }
    MYSQL_RES *rs = mysql_use_result(dbImpl->conn);
    if ((rs == NULL) && (mysql_errno(dbImpl->conn) != 0)) {
        stExcept *ex = createMySqlExcept(dbImpl, "query failed \"%0.60s\"", sql);
        stSafeCFree(sql);
        stThrow(ex);
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

// collect warnings into an array
static int getWarnings(MySqlDb *dbImpl, int maxToReport, char **warnings) {
    int numReturned = 0;
    MYSQL_RES *rs = queryStart(dbImpl, "show warnings limit %d", maxToReport);
    char **row;
    while ((row = queryNext(dbImpl, rs)) != NULL) {
        if (numReturned < maxToReport) {
            warnings[numReturned++] = stSafeCDynFmt("%s: %s (%s)", row[0], row[2], row[1]);
        }
    }
    mysql_free_result(rs);
    return numReturned;
}

// collect warnings, concat into a string
static char *getWarningsStr(MySqlDb *dbImpl) {
    static const int maxToReport = 5;
    char *warnings[maxToReport];
    int numReturned = getWarnings(dbImpl, maxToReport, warnings);
    char *warningsStr = stString_join("\n", (const char**)warnings, numReturned);
    for (int i = 0; i < numReturned; i++) {
        stSafeCFree(warnings[i]);
    }
    return warningsStr;
}

// and throw an expection
static void throwWarnings(MySqlDb *dbImpl) {
    int numWarnings = mysql_warning_count(dbImpl->conn);
    char *warningsStr = getWarningsStr(dbImpl);
    stExcept *ex = stExcept_new(ST_KV_DATABASE_EXCEPTION_ID, "MySQL request had %d warning(s), possible data lose: %s", numWarnings, warningsStr);
    stSafeCFree(warningsStr);
    stThrow(ex);
}

static void queryEnd(MySqlDb *dbImpl, MYSQL_RES *rs) {
    if (rs != NULL) {
        mysql_free_result(rs);
    }
    if (mysql_warning_count(dbImpl->conn) != 0) {
        throwWarnings(dbImpl);
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

/* disconnect and free MySqlDb object */
static void disconnect(MySqlDb *dbImpl) {
    if (dbImpl->conn != NULL) {
        mysql_close(dbImpl->conn);
    }
    stSafeCFree(dbImpl->table);
    stSafeCFree(dbImpl);
}

/* connect to a database server */
static MySqlDb *connect(stKVDatabaseConf *conf) {
    MySqlDb *dbImpl = stSafeCCalloc(sizeof(MySqlDb));
    if ((dbImpl->conn = mysql_init(NULL)) == NULL) {
        disconnect(dbImpl);
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "mysql_init failed");
    }
    if (mysql_real_connect(dbImpl->conn, stKVDatabaseConf_getHost(conf), stKVDatabaseConf_getUser(conf), stKVDatabaseConf_getPassword(conf), stKVDatabaseConf_getDatabaseName(conf), stKVDatabaseConf_getPort(conf), NULL, 0) == NULL) {
        stExcept *ex = createMySqlExcept(dbImpl, "failed to connect to MySql database: %s on %s as user %s",
                                         stKVDatabaseConf_getDatabaseName(conf), stKVDatabaseConf_getHost(conf), stKVDatabaseConf_getUser(conf));
        disconnect(dbImpl);
        stThrow(ex);
    }
    dbImpl->table = stString_copy(stKVDatabaseConf_getTableName(conf));

    // disable report of notes, so only warnings and errors come back
    sqlExec(dbImpl, "set sql_notes=0");

    // set max sizes of an sql statment to 1G.  This must also be specified
    // for the server by adding "max_allowed_packet = 1G" to the [mysqld]
    // section of my.cnf
    sqlExec(dbImpl, "set global max_allowed_packet=1073741824");

    // set the idle timeout to a week
    int waitTimeout = 7 * 24 * 60 * 60;  // 1 week
    sqlExec(dbImpl, "set wait_timeout=%d", waitTimeout);

    // set the read timeout to an hour
    int readTimeout = 60 * 60;  // 1 hour
    sqlExec(dbImpl, "set net_read_timeout=%d", readTimeout);

    // NOTE: commit will not return an error, this does row-level locking on
    // the select done before the update
    sqlExec(dbImpl, "set autocommit = 0;");
    sqlExec(dbImpl, "set session transaction isolation level serializable;");
    return dbImpl;
}

/* create the keyword/value table */
static void createKVTable(MySqlDb *dbImpl) {
    sqlExec(dbImpl, "drop table if exists %s", dbImpl->table);
    sqlExec(dbImpl, "create table %s (id bigint primary key, data longblob) engine=INNODB;", dbImpl->table);
}

static void destructDB(stKVDatabase *database) {
    MySqlDb *dbImpl = database->dbImpl;
    disconnect(dbImpl);
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

static bool containsRecord(stKVDatabase *database, int64_t key) {
    MySqlDb *dbImpl = database->dbImpl;
    MYSQL_RES *rs = queryStart(dbImpl, "select id from %s where id=%lld", dbImpl->table,  (long long)key);
    char **row = queryNext(dbImpl, rs);
    bool found = (row != NULL);
    queryEnd(dbImpl, rs);
    return found;
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

static void abortTransaction(stKVDatabase *database) {
    MySqlDb *dbImpl = database->dbImpl;
    sqlExec(dbImpl, "rollback;");
}

//initialisation function
void stKVDatabase_initialise_MySql(stKVDatabase *database, stKVDatabaseConf *conf, bool create) {
    database->dbImpl = connect(conf);
    database->destruct = destructDB;
    database->delete = deleteDB;
    database->containsRecord = containsRecord;
    database->insertRecord = insertRecord;
    database->updateRecord = updateRecord;
    database->numberOfRecords = numberOfRecords;
    database->getRecord = getRecord;
    database->getRecord2 = getRecord2;
    database->getPartialRecord = getPartialRecord;
    database->removeRecord = removeRecord;
    database->startTransaction = startTransaction;
    database->commitTransaction = commitTransaction;
    database->abortTransaction = abortTransaction;
    if (create) {
        createKVTable(database->dbImpl);
    }
}

#endif
