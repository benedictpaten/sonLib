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

static void getExpectedToken(char **tokenStream, const char *expected) {
    char *cA = stString_getNextWord(tokenStream);
    if(strcmp(cA, expected) != 0) {
        stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "BUG: expected the token: %s in database XML string, but I got: %s from the stream %s", expected, cA, *tokenStream);
    }
    free(cA);
}

static char *getKeyValue(char **tokenStream, const char *key) {
    getExpectedToken(tokenStream, key);
    return stString_getNextWord(tokenStream);
}

stKVDatabaseConf *stKVDatabaseConf_constructFromString(const char *xmlString) {
    stKVDatabaseConf *databaseConf;
    char *toReplace[7] = { "</", "<", "/>", ">", "=", "\"", "'" };
    char *cA = stString_replace(xmlString, toReplace[0], " "), *cA2;
    for(int32_t i=1; i<7; i++) {
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
        char *host = getKeyValue(&cA2, "host");
        char *portString = getKeyValue(&cA2, "port");
        uint32_t port;
        int32_t i = sscanf(portString, "%ui", &port);
        assert(i == 1);
        char *user = getKeyValue(&cA2, "user");
        char *password = getKeyValue(&cA2, "password");
        char *databaseName = getKeyValue(&cA2, "database_name");
        char *tableName = getKeyValue(&cA2, "table_name");
        databaseConf = stKVDatabaseConf_constructMySql(host, port, user, password, databaseName, tableName);
        free(host);
        free(portString);
        free(user);
        free(password);
        free(databaseName);
        free(tableName);
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

