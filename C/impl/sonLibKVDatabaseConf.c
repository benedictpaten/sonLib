/*
 * sonLibKVDatabaseConf.c
 *
 *  Created on: 2010-08-23
 *      Author: Mark Diekhans
 */
#include "sonLibKVDatabaseConf.h"
#include "sonLibKVDatabase.h"
#include "sonLibExcept.h"
#include "sonLibTypes.h"
#include "sonLibString.h"
#include "stSafeC.h"
#include "sonLib.h"

/* object storing the database configuration */
struct stKVDatabaseConf {
    stKVDatabaseType type;
    char *databaseDir;
    char *host;
    unsigned port;
    char *user;
    char *password;
    char *databaseName;
    char *tableName;
};

stKVDatabaseConf *stKVDatabaseConf_constructTokyoCabinet(
        const char *databaseDir) {
    stKVDatabaseConf *conf = stSafeCCalloc(sizeof(stKVDatabaseConf));
    conf->type = stKVDatabaseTypeTokyoCabinet;
    conf->databaseDir = stString_copy(databaseDir);
    return conf;
}

stKVDatabaseConf *stKVDatabaseConf_constructMySql(const char *host,
        unsigned port, const char *user, const char *password,
        const char *databaseName, const char *tableName) {
#ifndef HAVE_MYSQL
    stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
            "requested MySQL database, however sonlib is not compiled with MySql support");
#endif
    stKVDatabaseConf *conf = stSafeCCalloc(sizeof(stKVDatabaseConf));
    conf->type = stKVDatabaseTypeMySql;
    conf->host = stString_copy(host);
    conf->port = port;
    conf->user = stString_copy(user);
    conf->password = stString_copy(password);
    conf->databaseName = stString_copy(databaseName);
    conf->tableName = stString_copy(tableName);
    return conf;
}

char *getNextToken(char **tokenStream) {
    char *token = stString_getNextWord(tokenStream);
    if(token != NULL) {
        char *cA = stString_replace(token, "\"", "");
        free(token);
        token = stString_replace(cA, "'", "");
        free(cA);
    }
    return token;
}

static void getExpectedToken(char **tokenStream, const char *expected) {
    char *cA = getNextToken(tokenStream);
    if (strcmp(cA, expected) != 0) {
        stThrowNew(
                ST_KV_DATABASE_EXCEPTION_ID,
                "BUG: expected the token: %s in database XML string, but I got: %s from the stream %s",
                expected, cA, *tokenStream);
    }
    free(cA);
}

static char *getKeyValue(char **tokenStream, const char *key) {
    getExpectedToken(tokenStream, key);
    return getNextToken(tokenStream);
}

stKVDatabaseConf *stKVDatabaseConf_constructFromString(const char *xmlString) {
    stKVDatabaseConf *databaseConf;
    char *toReplace[5] = { "</", "<", "/>", ">", "=" };
    char *cA = stString_replace(xmlString, toReplace[0], " "), *cA2;
    for (int32_t i = 1; i < 5; i++) {
        cA2 = stString_replace(cA, toReplace[i], " ");
        free(cA);
        cA = cA2;
    }
    getExpectedToken(&cA2, "st_kv_database_conf");
    char *type = getKeyValue(&cA2, "type");
    if (strcmp(type, "tokyo_cabinet") == 0) {
        getExpectedToken(&cA2, "tokyo_cabinet");
        char *databaseDir = getKeyValue(&cA2, "database_dir");
        databaseConf = stKVDatabaseConf_constructTokyoCabinet(databaseDir);
        free(databaseDir);
    } else {
        assert(strcmp(type, "mysql") == 0);
        getExpectedToken(&cA2, "mysql");
        stHash *hash = stHash_construct3(stHash_stringKey, stHash_stringEqualKey,
                free, free);
        for (int32_t i = 0; i < 6; i++) {
            char *key = getNextToken(&cA2);
            char *value = getNextToken(&cA2);
            if (key == NULL || value == NULL) {
                stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                        "tried to get a key value pair from the MYSQL database conf string, but failed");
            }
            if (stHash_search(hash, key) != NULL) {
                stThrowNew(ST_KV_DATABASE_EXCEPTION_ID,
                        "got a duplicate entry in the MYSQL conf string");
            }
            stHash_insert(hash, key, value);
        }
        char *errorString = "did not find a %s value in the MYSQL string: %s";
        if (stHash_search(hash, "host") == NULL) {
            stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, errorString, "host", xmlString);
        }
        if (stHash_search(hash, "port") == NULL) {
            stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, errorString, "port", xmlString);
        }
        if (stHash_search(hash, "user") == NULL) {
            stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, errorString, "user", xmlString);
        }
        if (stHash_search(hash, "password") == NULL) {
            stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, errorString, "password", xmlString);
        }
        if (stHash_search(hash, "database_name") == NULL) {
            stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, errorString,
                    "database_name");
        }
        if (stHash_search(hash, "table_name") == NULL) {
            stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, errorString, "table_name", xmlString);
        }
        uint32_t port; //Parse the port integer
        int32_t i = sscanf(stHash_search(hash, "port"), "%ui", &port);
        assert(i == 1);
        databaseConf = stKVDatabaseConf_constructMySql(stHash_search(hash,
                "host"), port, stHash_search(hash, "user"), stHash_search(hash,
                "password"), stHash_search(hash, "database_name"),
                stHash_search(hash, "table_name"));
        stHash_destruct(hash);
    }
    free(type);
    free(cA);
    return databaseConf;
}

stKVDatabaseConf *stKVDatabaseConf_constructClone(stKVDatabaseConf *srcConf) {
    stKVDatabaseConf *conf = stSafeCCalloc(sizeof(stKVDatabaseConf));
    conf->type = stKVDatabaseTypeMySql;
    conf->databaseDir = stString_copy(srcConf->databaseDir);
    conf->host = stString_copy(srcConf->host);
    conf->port = srcConf->port;
    conf->user = stString_copy(srcConf->user);
    conf->password = stString_copy(srcConf->password);
    conf->databaseName = stString_copy(srcConf->databaseName);
    conf->tableName = stString_copy(srcConf->tableName);
    return conf;
}

void stKVDatabaseConf_destruct(stKVDatabaseConf *conf) {
    if (conf != NULL) {
        stSafeCFree(conf->databaseDir);
        stSafeCFree(conf->host);
        stSafeCFree(conf->user);
        stSafeCFree(conf->password);
        stSafeCFree(conf->databaseName);
        stSafeCFree(conf->tableName);
        stSafeCFree(conf);
    }
}

stKVDatabaseType stKVDatabaseConf_getType(stKVDatabaseConf *conf) {
    return conf->type;
}

const char *stKVDatabaseConf_getDir(stKVDatabaseConf *conf) {
    return conf->databaseDir;
}

const char *stKVDatabaseConf_getHost(stKVDatabaseConf *conf) {
    return conf->host;
}

unsigned stKVDatabaseConf_getPort(stKVDatabaseConf *conf) {
    return conf->port;
}

const char *stKVDatabaseConf_getUser(stKVDatabaseConf *conf) {
    return conf->user;
}

const char *stKVDatabaseConf_getPassword(stKVDatabaseConf *conf) {
    return conf->password;
}

const char *stKVDatabaseConf_getDatabaseName(stKVDatabaseConf *conf) {
    return conf->databaseName;
}

const char *stKVDatabaseConf_getTableName(stKVDatabaseConf *conf) {
    return conf->tableName;
}

