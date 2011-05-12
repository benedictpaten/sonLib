/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibKVDatabase_PostgreSql.c
 *
 *  Created on: 2010-11-17
 *      Author: Mark Diekhans
 */

#include "sonLibGlobalsInternal.h"
#include "sonLibKVDatabasePrivate.h"
#include "stSafeC.h"

#ifdef HAVE_POSTGRESQL

#include <libpq-fe.h>

/* PostgreSql client data object, stored in stKVDatabase object */
typedef struct {
    PGconn *conn;
    char *table;
} PgSqlDb;


static PGresult *queryStart(PgSqlDb *dbImpl, const char *query, ...)
#if defined(__GNUC__)
__attribute__((format(printf, 2, 3)))
#endif
;

static int sqlExec(PgSqlDb *dbImpl, const char *sqlCmd, ...)
#if defined(__GNUC__)
__attribute__((format(printf, 2, 3)))
#endif
;

/* is this a PostgreSql error were the transaction should be retried */
static bool isPgSqlRetryError(const PGresult *rs) {
    // libpq is painful on how one deals with checked in detailed sqlerror
    // code./ PQresultErrorField() gives you a string, however there is no
    // function to convert the the err code constants defined in
    // postgresql/server/utils/errcodes.h to something that can be compared.
    // So we look in errcodes-appendix.html and just hard code it.

    // check for class 40
    const char *sqlState = PQresultErrorField(rs, PG_DIAG_SQLSTATE);
    return (sqlState != NULL) && (strncmp(sqlState, "40", 2) == 0);

}

/* create an exception for the current PostgreSql conn error */
static stExcept *createPgSqlConnExceptv(PgSqlDb *dbImpl, const char *msg, va_list args) {
    char *fmtMsg = stSafeCDynFmtv(msg, args);
    stExcept *except = stExcept_new(ST_KV_DATABASE_EXCEPTION_ID, "%s: %s (%d)", fmtMsg, PQerrorMessage(dbImpl->conn), PQstatus(dbImpl->conn));
    stSafeCFree(fmtMsg);
    return except;
}

#if 0 // not used
/* create an exception for the current PostgreSql connn error */
static stExcept *createPgSqlConnExcept(PgSqlDb *dbImpl, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    stExcept *except = createPgSqlConnExceptv(dbImpl, msg, args);
    va_end(args);
    return except;
}
#endif

/* generate an exception for the current PostgreSql conn error */
static void throwPgSqlConnExcept(PgSqlDb *dbImpl, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    stExcept *except = createPgSqlConnExceptv(dbImpl, msg, args);
    va_end(args);
    stThrow(except);
}

/* create an exception for the current PostgreSql results error */
static stExcept *createPgSqlResultExceptv(PgSqlDb *dbImpl, const PGresult *rs, const char *msg, va_list args) {
    char *fmtMsg = stSafeCDynFmtv(msg, args);
    stExcept *except;
    if (rs == NULL) {
        except = stExcept_new(ST_KV_DATABASE_EXCEPTION_ID, "%s: fatal error (%d)", fmtMsg, PQerrorMessage(dbImpl->conn));
    } else {        
        const char *exId = isPgSqlRetryError(rs) ?  ST_KV_DATABASE_RETRY_TRANSACTION_EXCEPTION_ID : ST_KV_DATABASE_EXCEPTION_ID;
        except = stExcept_new(exId, "%s: %s", fmtMsg, PQresultErrorMessage(rs));
    }
    stSafeCFree(fmtMsg);
    return except;
}

/* create an exception for the current PostgreSql error */
static stExcept *createPgSqlResultExcept(PgSqlDb *dbImpl, const PGresult *result, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    stExcept *except = createPgSqlResultExceptv(dbImpl, result, msg, args);
    va_end(args);
    return except;
}

/* generate an exception for the current PostgreSql error */
static void throwPgSqlResultExcept(PgSqlDb *dbImpl, const PGresult *result, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    stExcept *except = createPgSqlResultExceptv(dbImpl, result, msg, args);
    va_end(args);
    stThrow(except);
}

/* notice receiver to turn warnings into errors */
static void noticeReceiver(void *arg, const PGresult *result) {
    PgSqlDb *dbImpl = arg;
    if (strncmp(PQresultErrorMessage(result), "WARNING:", 8) == 0) {
        throwPgSqlResultExcept(dbImpl, result, "warnings treated as errors");
    }
}

/* Execute SQL a non-select query, formatting arguments into the sql command.
 * returns number of rows affect or zero if that makes no sense */
static int sqlExecv(PgSqlDb *dbImpl, const char *sqlCmd, va_list args) {
    char *sql = stSafeCDynFmtv(sqlCmd, args);
    PGresult *rs = PQexec(dbImpl->conn, sql);
    if ((rs == NULL) || (PQresultStatus(rs) != PGRES_COMMAND_OK)) {
        stExcept *ex = createPgSqlResultExcept(dbImpl, rs, "query failed \"%0.60s\"", sql);
        stSafeCFree(sql);
        PQclear(rs);
        stThrow(ex);
    }
    const char *affected = PQcmdTuples(rs);
    int numRows = (strlen(affected) == 0) ? 0 : stSafeStrToInt64(affected);
    stSafeCFree(sql);
    PQclear(rs);
    return numRows;
}

/* start an SQL query, formatting arguments into the query */
static int sqlExec(PgSqlDb *dbImpl, const char *sqlCmd, ...) {
    va_list args;
    va_start(args, sqlCmd);
    int numRows = sqlExecv(dbImpl, sqlCmd, args);
    va_end(args);
    return numRows;
}

/* Execute SQL a non-select query, formatting arguments into the sql command */
static PGresult *queryStartv(PgSqlDb *dbImpl, const char *query, va_list args) {
    char *sql = stSafeCDynFmtv(query, args);
    PGresult *result = PQexec(dbImpl->conn, sql);
    if ((result == NULL) || (PQresultStatus(result) != PGRES_TUPLES_OK)) {
        stExcept *ex = createPgSqlResultExcept(dbImpl, result, "query failed \"%0.60s\"", sql);
        stSafeCFree(sql);
        PQclear(result);
        stThrow(ex);
    }
    stSafeCFree(sql);
    return result;
}

/* start an SQL query, formatting arguments into the query */
static PGresult *queryStart(PgSqlDb *dbImpl, const char *query, ...) {
    va_list args;
    va_start(args, query);
    PGresult *result = queryStartv(dbImpl, query, args);
    va_end(args);
    return result;
}

/* complain about not getting a single result tuple */
static stExcept *createExpectOneTuple(PgSqlDb *dbImpl, PGresult *rs, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    char *fmtMsg = stSafeCDynFmtv(msg, args);
    va_end(args);
    stExcept *except = stExcept_new(ST_KV_DATABASE_EXCEPTION_ID, "expected one tuple, got %d:" "%s", PQntuples(rs), fmtMsg);
    stSafeCFree(fmtMsg);
    return except;
}

static void queryEnd(PgSqlDb *dbImpl, PGresult *rs) {
    PQclear(rs);
}

/* SQL escape a byte string, result must be freed */
static unsigned char *sqlEscape(PgSqlDb *dbImpl, const void *data, size_t size) {
    size_t escStrLen;
    unsigned char *escStr = PQescapeByteaConn(dbImpl->conn, data, size, &escStrLen);
    if (escStr == NULL) {
        throwPgSqlConnExcept(dbImpl, "failed to allocate memory for encoding string");
    }
    return escStr;
}

/* SQL unescape a byte string, result must be freed */
static void *sqlUnescape(PgSqlDb *dbImpl, const char *escStr, size_t *size) {
    void *data = PQunescapeBytea((const unsigned char*)escStr, size);
    if (data == NULL) {
        throwPgSqlConnExcept(dbImpl, "failed to allocate memory for decoding string");
    }
    return data;
}

/* disconnect and free PgSqlDb object */
static void disconnect(PgSqlDb *dbImpl) {
    if (dbImpl->conn != NULL) {
        PQfinish(dbImpl->conn);
    }
    stSafeCFree(dbImpl->table);
    stSafeCFree(dbImpl);
}

/* get port as a string or NULL. WARNING: static return */
static const char *getPortStr(stKVDatabaseConf *conf) {
    static char portStrBuf[64];
    if (stKVDatabaseConf_getPort(conf) > 0) {
        stSafeCFmt(portStrBuf, sizeof(portStrBuf), "%d", stKVDatabaseConf_getPort(conf));
        return portStrBuf;
    } else {
        return NULL;
    }
} 

/* connect to a database server */
static PgSqlDb *connect(stKVDatabaseConf *conf) {
    PgSqlDb *dbImpl = stSafeCCalloc(sizeof(PgSqlDb));
    dbImpl->conn = PQsetdbLogin(stKVDatabaseConf_getHost(conf), getPortStr(conf), NULL, NULL,
                                stKVDatabaseConf_getDatabaseName(conf),
                                stKVDatabaseConf_getUser(conf), 
                                stKVDatabaseConf_getPassword(conf));
    if (PQstatus(dbImpl->conn) != CONNECTION_OK) {
        stExcept *ex = stExcept_new(ST_KV_DATABASE_EXCEPTION_ID, "failed to connect to PostgreSql database: %s on %s as user %s",
                                    stKVDatabaseConf_getDatabaseName(conf), stKVDatabaseConf_getHost(conf), stKVDatabaseConf_getUser(conf));
        disconnect(dbImpl);
        stThrow(ex);
    }
    dbImpl->table = stString_copy(stKVDatabaseConf_getTableName(conf));
    PQsetNoticeReceiver(dbImpl->conn, noticeReceiver, dbImpl);

    // NOTE: commit will not return an error, this does row-level locking on
    // the select done before the update
    sqlExec(dbImpl, "set session transaction isolation level serializable;");
    return dbImpl;
}

/* create the keyword/value table */
static void createKVTable(PgSqlDb *dbImpl) {
    sqlExec(dbImpl, "drop table if exists %s", dbImpl->table);
    sqlExec(dbImpl, "create table %s (id bigint primary key, data bytea);", dbImpl->table);
}

static void destructDB(stKVDatabase *database) {
    PgSqlDb *dbImpl = database->dbImpl;
    disconnect(dbImpl);
    database->dbImpl = NULL;
}

static void deleteDB(stKVDatabase *database) {
    PgSqlDb *dbImpl = database->dbImpl;
    sqlExec(dbImpl, "drop table %s", dbImpl->table);
    destructDB(database);
}

static void insertRecord(stKVDatabase *database, int64_t key, const void *value, int64_t sizeOfRecord) {
    PgSqlDb *dbImpl = database->dbImpl;
    unsigned char *buf = sqlEscape(dbImpl, value, sizeOfRecord);
    sqlExec(dbImpl, "insert into %s (id, data) values (%lld, E'%s')", dbImpl->table, (long long)key, buf);
    stSafeCFree(buf);
}

static void updateRecord(stKVDatabase *database, int64_t key,
                         const void *value, int64_t sizeOfRecord) {
    PgSqlDb *dbImpl = database->dbImpl;
    unsigned char *buf = sqlEscape(dbImpl, value, sizeOfRecord);
    sqlExec(dbImpl, "update %s set data=E'%s' where id=%lld", dbImpl->table, buf,  (long long)key);
    stSafeCFree(buf);
}

static int64_t numberOfRecords(stKVDatabase *database) {
    PgSqlDb *dbImpl = database->dbImpl;
    const char *sqlTmpl = "select count(*) from %s";
    PGresult *rs = queryStart(dbImpl, sqlTmpl, dbImpl->table);
    if (PQntuples(rs) != 1) {
        stExcept *ex = createExpectOneTuple(dbImpl, rs, sqlTmpl, dbImpl->table);
        queryEnd(dbImpl, rs);
        stThrow(ex);
    }
    int64_t numRecords = stSafeStrToInt64(PQgetvalue(rs, 0, 0));
    queryEnd(dbImpl, rs);
    return numRecords;
}

static void *getRecord2(stKVDatabase *database, int64_t key, int64_t *sizeOfRecord) {
    PgSqlDb *dbImpl = database->dbImpl;
    PGresult *rs = queryStart(dbImpl, "select data from %s where id=%lld", dbImpl->table,  (long long)key);
    void *data = NULL;
    size_t readLen = 0;
    if (PQntuples(rs) > 0) {
        data = sqlUnescape(dbImpl, PQgetvalue(rs, 0, 0), &readLen);
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
    PgSqlDb *dbImpl = database->dbImpl;
    PGresult *rs = queryStart(dbImpl, "select id from %s where id=%lld", dbImpl->table,  (long long)key);
    bool found = PQntuples(rs) > 0;
    queryEnd(dbImpl, rs);
    return found;
}

static void *getPartialRecord(stKVDatabase *database, int64_t key, int64_t zeroBasedByteOffset, int64_t sizeInBytes, int64_t recordSize) {
    PgSqlDb *dbImpl = database->dbImpl;
    PGresult *rs = queryStart(dbImpl, "select substring(data, %lld, %lld) from %s where id=%lld", (long long)zeroBasedByteOffset+1, (long long)sizeInBytes, dbImpl->table, (long long)key);
    void *data = NULL;
    size_t readLen = 0;
    if (PQntuples(rs) > 0) {
        data = sqlUnescape(dbImpl, PQgetvalue(rs, 0, 0), &readLen);
    }
    queryEnd(dbImpl, rs);
    if (readLen != sizeInBytes) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "partial read of key %lld, expected %lld bytes got %lld bytes", (long long)key, (long long)sizeInBytes, (long long)readLen);
    }
    return data;
}

static void removeRecord(stKVDatabase *database, int64_t key) {
    PgSqlDb *dbImpl = database->dbImpl;
    int numRows = sqlExec(dbImpl, "delete from %s where id=%lld", dbImpl->table, (long long)key);
    if (numRows == 0) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "remove of non-existent key %lld", (long long)key);
    }
}

static void startTransaction(stKVDatabase *database) {
    PgSqlDb *dbImpl = database->dbImpl;
    sqlExec(dbImpl, "start transaction isolation level serializable;");
}

static void commitTransaction(stKVDatabase *database) {
    PgSqlDb *dbImpl = database->dbImpl;
    sqlExec(dbImpl, "commit;");
}

static void abortTransaction(stKVDatabase *database) {
    PgSqlDb *dbImpl = database->dbImpl;
    sqlExec(dbImpl, "rollback;");
}

static int64_t incrementRecord(stKVDatabase *database, int64_t key, int64_t incrementAmount) {
    startTransaction(database);
    int64_t returnValue = INT64_MIN;
    stTry {
        int64_t recordSize;
        int64_t *record = getRecord2(database, key, &recordSize);
        assert(recordSize >= sizeof(int64_t));
        record[0] += incrementAmount;
        returnValue = record[0];
        updateRecord(database, key, record, recordSize);
        free(record);
        commitTransaction(database);
    }stCatch(ex) {
        abortTransaction(database);
        stThrowNewCause(
                ex,
                ST_KV_DATABASE_EXCEPTION_ID,
                "MySQL increment record failed");
    }stTryEnd;
    return returnValue;
}

// TODO: make this one command -- faster, and watch out for maxlength
static void bulkRemoveRecords(stKVDatabase *database, stList *records) {
    startTransaction(database);
    stTry {
        for(int32_t i=0; i<stList_length(records); i++) {
            stInt64Tuple *j = stList_get(records, i);
            removeRecord(database, stInt64Tuple_getPosition(j, 0));
        }
        commitTransaction(database);
    }stCatch(ex) {
        abortTransaction(database);
        stThrowNewCause(
                ex,
                ST_KV_DATABASE_EXCEPTION_ID,
                "MySQL bulk remove records failed");
    }stTryEnd;
}

static void setRecord(stKVDatabase *database, int64_t key,
                         const void *value, int64_t sizeOfRecord) {

    if (containsRecord(database, key)) {
        updateRecord(database, key, value, sizeOfRecord);
    } else {
        insertRecord(database, key, value, sizeOfRecord);
    }
}

// TODO: see if this can be made into one compound command
static void bulkSetRecords(stKVDatabase *database, stList *records) {
    startTransaction(database);
    stTry {
        for(int32_t i=0; i<stList_length(records); i++) {
            stKVDatabaseBulkRequest *request = stList_get(records, i);
            switch(request->type) {
                case UPDATE:
                updateRecord(database, request->key, request->value, request->size);
                break;
                case INSERT:
                insertRecord(database, request->key, request->value, request->size);
                break;
                case SET:
                setRecord(database, request->key, request->value, request->size);
                break;
            }
        }
        commitTransaction(database);
    }stCatch(ex) {
        abortTransaction(database);
        stThrowNewCause(
                ex,
                ST_KV_DATABASE_EXCEPTION_ID,
                "MySQL bulk set records failed");
    }stTryEnd;
}

// initialisation function
void stKVDatabase_initialise_PostgreSql(stKVDatabase *database, stKVDatabaseConf *conf, bool create) {
    database->dbImpl = connect(conf);
    database->destruct = destructDB;
    database->deleteDatabase = deleteDB;
    database->containsRecord = containsRecord;
    database->insertRecord = insertRecord;
    database->updateRecord = updateRecord;
    database->setRecord = setRecord;
    database->incrementRecord = incrementRecord;
    database->bulkSetRecords = bulkSetRecords;
    database->bulkRemoveRecords = bulkRemoveRecords;
    database->numberOfRecords = numberOfRecords;
    database->getRecord = getRecord;
    database->getRecord2 = getRecord2;
    database->getPartialRecord = getPartialRecord;
    database->removeRecord = removeRecord;
    if (create) {
        createKVTable(database->dbImpl);
    }
}

#endif
