/* Copyright (c) 2017-2018 Snowflake Computing Inc. All right reserved.  */

#ifndef PHP_PDO_SNOWFLAKE_INT_H
#define PHP_PDO_SNOWFLAKE_INT_H

#include <snowflake/client.h>
#include <snowflake/logger.h>
#include "snowflake_arraylist.h"

/**
 * PHP PDO Snowflake Driver name
 */
#define PHP_PDO_SNOWFLAKE_NAME "PDO"

#define PDO_DBG_INF(...) sf_log_debug(PHP_PDO_SNOWFLAKE_NAME, __VA_ARGS__)
#define PDO_DBG_ERR(...) sf_log_error(PHP_PDO_SNOWFLAKE_NAME, __VA_ARGS__)
#define PDO_DBG_ENTER(func_name) sf_log_trace(PHP_PDO_SNOWFLAKE_NAME, "Entering: %s", func_name)
#define PDO_DBG_RETURN(value)  do { sf_log_trace(PHP_PDO_SNOWFLAKE_NAME, "Leaving: %d", value); return (value); } while (0)

/**
 * Snowflake module global variables.
 */
ZEND_BEGIN_MODULE_GLOBALS(pdo_snowflake)
    char *cacert; /* location of cacert.pem */
    char *logdir; /* log directory */
    char *loglevel; /* log level */
ZEND_END_MODULE_GLOBALS(pdo_snowflake)

ZEND_EXTERN_MODULE_GLOBALS(pdo_snowflake)

#if (PHP_VERSION_ID >= 70000)
#   define PDO_SNOWFLAKE_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(pdo_snowflake, v)
#   if defined(ZTS) && defined(COMPILE_DL_PDO_SNOWFLAKE)
ZEND_TSRMLS_CACHE_EXTERN()
#   endif
#else
typedef long zend_long;
#endif /* PHP_VERSION_ID */

typedef struct {
    SF_CONNECT *server;
} pdo_snowflake_db_handle;

typedef struct {
    pdo_snowflake_db_handle *H;
    SF_STMT *stmt;

    ARRAY_LIST *bound_params;
    SF_BIND_OUTPUT *bound_result;
} pdo_snowflake_stmt;

extern pdo_driver_t pdo_snowflake_driver;
extern struct pdo_stmt_methods snowflake_stmt_methods;

extern int
_pdo_snowflake_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, const char *file,
                     int line);

#define pdo_snowflake_error(d) _pdo_snowflake_error(d, NULL, __FILE__, __LINE__)
#define pdo_snowflake_error_stmt(s) _pdo_snowflake_error(s->dbh, s, __FILE__, __LINE__)

enum {
    PDO_SNOWFLAKE_ATTR_SSL_CAPATH = PDO_ATTR_DRIVER_SPECIFIC,
    PDO_SNOWFLAKE_ATTR_SSL_VERSION,
    PDO_SNOWFLAKE_ATTR_SSL_VERIFY_CERTIFICATE_REVOCATION_STATUS
};

#endif /* PHP_PDO_SNOWFLAKE_INT_H */
