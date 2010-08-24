#ifndef SONLIBKVDATABASECONF_H
#define SONLIBKVDATABASECONF_H
#include "sonLibTypes.h"

typedef enum {
    stKVDatabaseTypeTokyoCabinet,
    stKVDatabaseTypeMySql
} stKVDatabaseType;

/* 
 * Construct a new database configuration object for a Tokyo Cabinet
 * database.
 */
stKVDatabaseConf *stKVDatabaseConf_constructTokyoCabinet(const char *databaseDir);

/* 
 * Construct a new database configuration object for a MySql database.
 * password maybe NULL for no password.
 * port maybe 0 for the default port.
 */
stKVDatabaseConf *stKVDatabaseConf_constructMySql(const char *host, unsigned port, const char *user, const char *password,
                                                  const char *databaseName, const char *tableName);

/* 
 * Construct a new database configuration from an existing one.
 */
stKVDatabaseConf *stKVDatabaseConf_constructClone(stKVDatabaseConf *srcConf);

/*
 * Free the object.
 */
void stKVDatabaseConf_destruct(stKVDatabaseConf *conf);

/* 
 * get the database type.
 */
stKVDatabaseType stKVDatabaseConf_getType(stKVDatabaseConf *conf);

/* get the directory for file based databases */
const char *stKVDatabaseConf_getDir(stKVDatabaseConf *conf);

/* get the host for server based databases */
const char *stKVDatabaseConf_getHost(stKVDatabaseConf *conf);

/* get the port for server based databases */
unsigned stKVDatabaseConf_getPort(stKVDatabaseConf *conf);

/* get the user for server based databases */
const char *stKVDatabaseConf_getUser(stKVDatabaseConf *conf);

/* get the password for server based databases */
const char *stKVDatabaseConf_getPassword(stKVDatabaseConf *conf);

/* get the SQL database for server based databases */
const char *stKVDatabaseConf_getDatabaseName(stKVDatabaseConf *conf);

/* get the table name for server based databases */
const char *stKVDatabaseConf_getTableName(stKVDatabaseConf *conf);

#endif
