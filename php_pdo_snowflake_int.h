/* Copyright (c) 2017 Snowflake Computing Inc. All right reserved.  */

#ifndef PHP_PDO_SNOWFLAKE_INT_H
#define PHP_PDO_SNOWFLAKE_INT_H

#include <snowflake_client.h>

#if 1
#define PDO_DBG_ENABLED 1

#define PDO_DBG_INF(...) pdo_snowflake_log(__LINE__, __FILE__, "INFO", __VA_ARGS__)
#define PDO_DBG_ERR(...) pdo_snowflake_log(__LINE__, __FILE__, "ERROR", __VA_ARGS__)
#define PDO_DBG_ENTER(func_name) pdo_snowflake_log(__LINE__, __FILE__, "E", func_name)
#define PDO_DBG_RETURN(value)  do { pdo_snowflake_log(__LINE__, __FILE__, "R", ""); return (value); } while (0)
#define PDO_DBG_VOID_RETURN(value)  do { pdo_snowflake_log(__LINE__, __FILE__, "R", ""); return; } while (0)
#else
#define PDO_DBG_ENABLED 0
static inline void PDO_DBG_INF(char *format, ...) {}
static inline void PDO_DBG_ERR(char *format, ...) {}
static inline void PDO_DBG_ENTER(char *func_name) {}
#define PDO_DBG_RETURN(value)	return (value)
#define PDO_DBG_VOID_RETURN		return;
#endif

/**
 * Snowflake module global variables.
 */
ZEND_BEGIN_MODULE_GLOBALS(pdo_snowflake)
#if PDO_DBG_ENABLED
    char *debug; /* The actual string */
#endif
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
    SNOWFLAKE *server;
} pdo_snowflake_db_handle;

typedef struct {
    pdo_snowflake_db_handle *H;
    SNOWFLAKE_STMT *stmt;

    int64 num_params;
    SNOWFLAKE_BIND_INPUT *bound_params;
    SNOWFLAKE_BIND_OUTPUT *bound_result;
    sf_bool *out_null; /* TODO: need this? */
    zend_ulong *out_length; /* TODO: need this? */
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

/* TODO: adhoc logger until Snowflake Client provides it. */
extern void
pdo_snowflake_log(int line, const char *filename, const char *severity,
                  char *fmt, ...);

#endif /* PHP_PDO_SNOWFLAKE_INT_H */
