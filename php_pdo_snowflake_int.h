#ifndef PHP_PDO_SNOWFLAKE_INT_H
#define PHP_PDO_SNOWFLAKE_INT_H

#include <snowflake/client.h>
#include <snowflake/logger.h>
#include "snowflake_paramstore.h"

/**
 * PHP PDO Snowflake Driver name
 */
#define PHP_PDO_SNOWFLAKE_NAME "PDO"

#define PDO_LOG_INF(...) sf_log_info(PHP_PDO_SNOWFLAKE_NAME, __VA_ARGS__)
#define PDO_LOG_DBG(...) sf_log_debug(PHP_PDO_SNOWFLAKE_NAME, __VA_ARGS__)
#define PDO_LOG_ERR(...) sf_log_error(PHP_PDO_SNOWFLAKE_NAME, __VA_ARGS__)
#define PDO_LOG_ENTER(func_name) sf_log_trace(PHP_PDO_SNOWFLAKE_NAME, "Entering: %s", func_name)
#define PDO_LOG_RETURN(value)  do { sf_log_trace(PHP_PDO_SNOWFLAKE_NAME, "Leaving: %d", value); return (value); } while (0)

/**
 * Snowflake module global variables.
 */
ZEND_BEGIN_MODULE_GLOBALS(pdo_snowflake)
    char *cacert; /* location of cacert.pem */
    char *logdir; /* log directory */
    char *loglevel; /* log level */
    char *debug; /* debug flag. This dumps all logs on screen */
    char *clientconfigfile; /*location of client config file*/
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
    char *value;
    size_t size;
} pdo_snowflake_string;

typedef struct {
    SF_CONNECT *server;
    char last_qid[SF_UUID4_LEN];
} pdo_snowflake_db_handle;

typedef struct {
    pdo_snowflake_db_handle *H;
    SF_STMT *stmt;

    void *bound_params;
    pdo_snowflake_string *bound_results;
} pdo_snowflake_stmt;

extern pdo_driver_t pdo_snowflake_driver;
extern struct pdo_stmt_methods snowflake_stmt_methods;

extern int
_pdo_snowflake_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, const char *file,
                     int line);

extern void *_pdo_snowflake_user_realloc(void* org_ptr, size_t new_size);
extern void *_pdo_snowflake_user_calloc(size_t nitems, size_t size);
extern void *_pdo_snowflake_user_malloc(size_t size);
extern void _pdo_snowflake_user_dealloc(void* ptr);

#define pdo_snowflake_error(d) _pdo_snowflake_error(d, NULL, __FILE__, __LINE__)
#define pdo_snowflake_error_stmt(s) _pdo_snowflake_error(s->dbh, s, __FILE__, __LINE__)

enum {
    PDO_SNOWFLAKE_ATTR_SSL_CAPATH = PDO_ATTR_DRIVER_SPECIFIC,
    PDO_SNOWFLAKE_ATTR_SSL_VERSION,
    PDO_SNOWFLAKE_ATTR_SSL_VERIFY_CERTIFICATE_REVOCATION_STATUS,
    PDO_SNOWFLAKE_ATTR_QUERY_ID
};

#define PDO_SNOWFLAKE_CONN_ATTR_HOST_IDX 0
#define PDO_SNOWFLAKE_CONN_ATTR_PORT_IDX 1
#define PDO_SNOWFLAKE_CONN_ATTR_ACCOUNT_IDX 2
#define PDO_SNOWFLAKE_CONN_ATTR_REGION_IDX 3
#define PDO_SNOWFLAKE_CONN_ATTR_DATABASE_IDX 4
#define PDO_SNOWFLAKE_CONN_ATTR_SCHEMA_IDX 5
#define PDO_SNOWFLAKE_CONN_ATTR_WAREHOUSE_IDX 6
#define PDO_SNOWFLAKE_CONN_ATTR_ROLE_IDX 7
#define PDO_SNOWFLAKE_CONN_ATTR_PROTOCOL_IDX 8
#define PDO_SNOWFLAKE_CONN_ATTR_INSECURE_MODE_IDX 9
#define PDO_SNOWFLAKE_CONN_ATTR_TIMEZONE_IDX 10
#define PDO_SNOWFLAKE_CONN_ATTR_APPLICATION_IDX 11
#define PDO_SNOWFLAKE_CONN_ATTR_AUTHENTICATOR_IDX 12
#define PDO_SNOWFLAKE_CONN_ATTR_PRIV_KEY_FILE_IDX 13
#define PDO_SNOWFLAKE_CONN_ATTR_PRIV_KEY_FILE_PWD_IDX 14
#define PDO_SNOWFLAKE_CONN_ATTR_PROXY_IDX 15
#define PDO_SNOWFLAKE_CONN_ATTR_NO_PROXY_IDX 16
#define PDO_SNOWFLAKE_CONN_ATTR_DISABLE_QUERY_CONTEXT_CACHE_IDX 17
#define PDO_SNOWFLAKE_CONN_ATTR_INCLUDE_RETRY_REASON_IDX 18
#define PDO_SNOWFLAKE_CONN_ATTR_LOGIN_TIMEOUT_IDX 19
#define PDO_SNOWFLAKE_CONN_ATTR_MAX_RETRIES_IDX 20
#define PDO_SNOWFLAKE_CONN_ATTR_RETRY_TIMEOUT_IDX 21
#define PDO_SNOWFLAKE_CONN_ATTR_OCSP_FAIL_OPEN_IDX 22
#define PDO_SNOWFLAKE_CONN_ATTR_OCSP_DISABLE_IDX 23
#define PDO_SNOWFLAKE_CONN_ATTR_PASSCODE_IDX 24
#define PDO_SNOWFLAKE_CONN_ATTR_PASSCODE_IN_PASSWORD_IDX 25
#define PDO_SNOWFLAKE_CONN_ATTR_DISABLE_SAML_URL_CHECK_IDX 26
#define PDO_SNOWFLAKE_CONN_ATTR_CRL_CHECK_IDX 27
#define PDO_SNOWFLAKE_CONN_ATTR_CRL_ADVISORY_IDX 28
#define PDO_SNOWFLAKE_CONN_ATTR_CRL_ALLOW_NO_CRL_IDX 29
#define PDO_SNOWFLAKE_CONN_ATTR_CRL_MEMORY_CACHING_IDX 30
#define PDO_SNOWFLAKE_CONN_ATTR_CRL_DISK_CACHING_IDX 31
#define PDO_SNOWFLAKE_CONN_ATTR_CRL_DOWNLOAD_TIMEOUT_IDX 32
#define PDO_SNOWFLAKE_CONN_ATTR_OAUTH_TOKEN_ENDPOINT 33
#define PDO_SNOWFLAKE_CONN_ATTR_OAUTH_AUTHORIZATION_ENDPOINT 34
#define PDO_SNOWFLAKE_CONN_ATTR_OAUTH_REDIRECT_URI 35
#define PDO_SNOWFLAKE_CONN_ATTR_OAUTH_CLIENT_ID 36
#define PDO_SNOWFLAKE_CONN_ATTR_OAUTH_CLIENT_SECRET 37
#define PDO_SNOWFLAKE_CONN_ATTR_OAUTH_SCOPE 38
#define PDO_SNOWFLAKE_CONN_ATTR_SINGLE_USE_REFRESH_TOKEN 39
#define PDO_SNOWFLAKE_CONN_ATTR_WIF_PROVIDER_IDX 40
#define PDO_SNOWFLAKE_CONN_ATTR_WIF_TOKEN_IDX 41
#define PDO_SNOWFLAKE_CONN_ATTR_WIF_AZURE_RESOURCE_IDX 42
#define PDO_SNOWFLAKE_CONN_ATTR_CLIENT_STORE_TEMPORARY_CREDENTIAL 43
#define PDO_SNOWFLAKE_CONN_ATTR_CLIENT_REQUEST_MFA_TOKEN 44

#endif /* PHP_PDO_SNOWFLAKE_INT_H */
