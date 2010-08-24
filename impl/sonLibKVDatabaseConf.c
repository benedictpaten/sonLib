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

stKVDatabaseConf *stKVDatabaseConf_constructTokyoCabinet(const char *databaseDir) {
    stKVDatabaseConf *conf = stSafeCCalloc(sizeof(stKVDatabaseConf));
    conf->type = stKVDatabaseTypeTokyoCabinet;
    conf->databaseDir = stString_copy(databaseDir);
    return conf;
}

stKVDatabaseConf *stKVDatabaseConf_constructMySql(const char *host, unsigned port, const char *user, const char *password,
                                                  const char *databaseName, const char *tableName) {
#ifndef HAVE_MYSQL
    stThrowNew(ST_KV_DATABASE_EXCEPTION_ID, "requested MySQL database, however sonlib is not compiled with MySql support");
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

