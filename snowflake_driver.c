#ifdef HAVE_CONFIG_H
#endif

#include "php.h"
#include "pdo/php_pdo_driver.h"
#include "php_pdo_snowflake.h"
#include "php_pdo_snowflake_int.h"
#include "Zend/zend_exceptions.h"

void *_pdo_snowflake_user_realloc(void* org_ptr, size_t new_size) /* {{{ */
{
    return erealloc(org_ptr, new_size);
}
/* }}} */

void *_pdo_snowflake_user_calloc(size_t nitems, size_t size) /* {{{ */
{
    return ecalloc(nitems, size);
}
/* }}} */

void *_pdo_snowflake_user_malloc(size_t size) /* {{{ */
{
    return emalloc(size);
}
/* }}} */

void _pdo_snowflake_user_dealloc(void* ptr) /* {{{ */
{
    efree(ptr);
}


int _pdo_snowflake_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, const char *file,
                         int line) /* {{{ */
{
    pdo_snowflake_db_handle *H = (pdo_snowflake_db_handle *) dbh->driver_data;
    pdo_error_type *pdo_err;
    SF_ERROR_STRUCT *einfo;
    pdo_snowflake_stmt *S = NULL;

    PDO_LOG_ENTER("_pdo_snowflake_error");
    PDO_LOG_DBG("file=%s line=%d", file, line);
    if (stmt) {
        PDO_LOG_ERR("stmt error");
        S = (pdo_snowflake_stmt *) stmt->driver_data;
        pdo_err = &stmt->error_code;
        einfo = &S->stmt->error;
    } else {
        PDO_LOG_ERR("connection error");
        pdo_err = &dbh->error_code;
        einfo = &H->server->error;
    }
    PDO_LOG_ERR("error code: %ld", einfo->error_code);

    /* Adjust the file and line to reference to PDO source code instead of
     * Snowflake Client code. */
    einfo->file = (char *) file;
    einfo->line = line;

    if (!einfo->error_code) {
        /* No error if the error code is 0 */
        strcpy(*pdo_err, PDO_ERR_NONE);
        PDO_LOG_RETURN(0);
    }

    /* Set SQLSTATE */
    strcpy(*pdo_err, einfo->sqlstate);
    PDO_LOG_ERR("sqlstate: %s, msg: %s", pdo_err, einfo->msg);

    if (!dbh->methods) {
        PDO_LOG_ERR("Failed to allocate DBH");
        // If the error occurs when intializing dbh, always raise an exception
        zend_throw_exception_ex(
            php_pdo_get_exception(),
            einfo->error_code, // driver specific error code
            "SQLSTATE[%s] [%d] %s",
            *pdo_err, einfo->error_code, einfo->msg);
    }
    PDO_LOG_RETURN(1);
}

/* }}} */

/**
 * Get fetch error information.  if stmt is not null, fetch information
 * pertaining to the statement, otherwise fetch global error information.
 * The driver should add the following information to the array "info" in
 * this order:
 * - native error code
 * - string representation of the error code ... any other optional driver
 *   specific data ...
 *
 * @param dbh Pointer to the database handle initialized by the handle factory
 * @param stmt Pointer to the returned statement or NULL if an error occurs.
 * @param info an array of error info buffer
 * @return 1 if success or 0 if error occurs
 */
static int pdo_snowflake_fetch_error_func(
    pdo_dbh_t *dbh, pdo_stmt_t *stmt, zval *info) /* {{{ */
{
    pdo_snowflake_db_handle *H = (pdo_snowflake_db_handle *) dbh->driver_data;
    SF_ERROR_STRUCT *einfo = NULL;

    PDO_LOG_ENTER("pdo_snowflake_fetch_error_func");
    PDO_LOG_DBG("dbh=%p stmt=%p", dbh, stmt);
    if (stmt) {
        pdo_snowflake_stmt *S = (pdo_snowflake_stmt *) stmt->driver_data;
        einfo = &S->stmt->error;
    } else {
        einfo = &H->server->error;
    }
    if (einfo->error_code) {
        // errorInfo[1]
        add_next_index_long(info, einfo->error_code);
        // errorInfo[2]
        add_next_index_string(info, einfo->msg);
    }
    PDO_LOG_RETURN(1);
}
/* }}} */

/**
 * Close an opened database.
 *
 * @param dbh Pointer to the database handle initialized by the handle factory
 * @return 1 if success or 0 if error occurs
 */
static int snowflake_handle_closer(pdo_dbh_t *dbh) /* {{{ */
{
    PDO_LOG_ENTER("snowflake_handle_closer");
    pdo_snowflake_db_handle *H = (pdo_snowflake_db_handle *) dbh->driver_data;

    if (H) {
        if (H->server) {
            snowflake_term(H->server);
            H->server = NULL;
        }
        pefree(H, dbh->is_persistent);
        dbh->driver_data = NULL;
    }
    PDO_LOG_RETURN(1);
}
/* }}} */

/**
 * Prepare raw SQL for execution, storing whatever state is appropriate into
 * the stmt that is passed in called by PDO in response to PDO::query() and
 * PDO::prepare()
 *
 * This function is essentially the constructor for a stmt object. This
 * function is responsible for processing statement options, and setting
 * driver-specific option fields in the pdo_stmt_t structure.
 *
 * PDO does not process any statement options on the driver's behalf before
 * calling the preparer function. It is your responsibility to process them
 * before you return, raising an error for any unknown options that are passed.
 *
 * One very important responsibility of this function is the processing of
 * SQL statement parameters. At the time of this call, PDO does not know if
 * your driver supports binding parameters into prepared statements, nor does
 * it know if it supports named or positional parameter naming conventions.
 *
 * Your driver is responsible for setting stmt->supports_placeholders as
 * appropriate for the underlying database. This may involve some run-time
 * determination on the part of your driver, if this setting depends on the
 * version of the database server to which it is connected. If your driver
 * doesn't directly support both named and positional parameter conventions,
 * you should use the pdo_parse_params() API to have PDO rewrite the query to
 * take advantage of the support provided by your database.
 *
 * @param dbh Pointer to the database handle initialized by the handle factory
 * @param sql Pointer to a character string containing the SQL statement to be prepared.
 * @param sql_len The length of the SQL statement.
 * @param stmt Pointer to the returned statement or NULL if an error occurs.
 * @param driver_options Any driver specific/defined options.
 * @return 1 if success or 0 if error occurs
 */
static int
snowflake_handle_preparer(pdo_dbh_t *dbh, const char *sql, size_t sql_len,
                          pdo_stmt_t *stmt, zval *driver_options) /* {{{ */
{
    PDO_LOG_ENTER("snowflake_handle_preparer");
    PDO_LOG_DBG("dbh=%p", dbh);
    PDO_LOG_DBG("sql=%.*s, len=%ld", (int) sql_len, sql, sql_len);
    pdo_snowflake_db_handle *H = (pdo_snowflake_db_handle *) dbh->driver_data;

    /* allocate PDO stmt */
    pdo_snowflake_stmt *S = ecalloc(1, sizeof(pdo_snowflake_stmt));

    S->H = H;
    stmt->driver_data = S;
    stmt->methods = &snowflake_stmt_methods;

    stmt->supports_placeholders = PDO_PLACEHOLDER_POSITIONAL | PDO_PLACEHOLDER_NAMED;

    /* allocate Snowflake stmt. Must be freed in dtor */
    if (!(S->stmt = snowflake_stmt(H->server))) {
        pdo_snowflake_error(dbh);
        PDO_LOG_RETURN(0);
    }

    if (snowflake_stmt_set_attr(S->stmt, SF_STMT_USER_REALLOC_FUNC, _pdo_snowflake_user_realloc) != SF_STATUS_SUCCESS) {
        pdo_snowflake_error_stmt(stmt);
        PDO_LOG_RETURN(0);
    }

    /* prepare SQL */
    if (snowflake_prepare(S->stmt, sql, sql_len) != SF_STATUS_SUCCESS) {
        pdo_snowflake_error_stmt(stmt);
        PDO_LOG_RETURN(0);
    }
    dbh->alloc_own_columns = 1;

    PDO_LOG_RETURN(1);
}
/* }}} */

/**
 * Execute a raw SQL statement. No pdo_stmt_t is created.
 *
 * @param dbh Pointer to the database handle initialized by the handle factory
 * @param sql Pointer to a character string containing the SQL statement to be prepared.
 * @param sql_len The length of the SQL statement.
 * @return the number of affected rows or -1 if an error occurs
 */
static zend_long
snowflake_handle_doer(pdo_dbh_t *dbh, const char *sql, size_t sql_len) /* {{{ */
{
    PDO_LOG_ENTER("snowflake_handle_doer");
    int ret = 0;
    SF_STATUS query_status;
    const char * qid;
    pdo_snowflake_db_handle *H = (pdo_snowflake_db_handle *) dbh->driver_data;
    PDO_LOG_DBG("sql: %.*s, len: %d", sql_len, sql, sql_len);
    SF_STMT *sfstmt = snowflake_stmt(H->server);

    // set realloc function for large size result
    snowflake_stmt_set_attr(sfstmt, SF_STMT_USER_REALLOC_FUNC,
                            _pdo_snowflake_user_realloc);

    query_status = snowflake_query(sfstmt, sql, sql_len);

    //save query id if available
    qid = snowflake_sfqid(sfstmt);
    if (qid && (strlen(qid) > 0))
    {
        strncpy(H->last_qid, qid, sizeof(H->last_qid) - 1);
    }

    if (query_status == SF_STATUS_SUCCESS) {
        int64 rows = snowflake_affected_rows(sfstmt);
        if (rows == -1) {
            snowflake_propagate_error(H->server, sfstmt);
            pdo_snowflake_error(dbh);
            ret = -1;
            goto cleanup;
        }
        // return number of rows affected
        ret = (int) rows;
    } else {
        snowflake_propagate_error(H->server, sfstmt);
        pdo_snowflake_error(dbh);
        ret = -1;
        goto cleanup;
    }

cleanup:
    snowflake_stmt_term(sfstmt);

    PDO_LOG_RETURN(ret);
}
/* }}} */

/**
 * Retrieve the ID of the last inserted row.
 *
 * Not implemented as Snowflake doesn't support the last inserted ID.
 * @param dbh Pointer to the database handle initialized by the handle factory
 * @param name string representing a table or sequence name.
 * @param len the length of the name parameter.
 * @return 1 if success or 0 if error occurs
 */
static char *pdo_snowflake_last_insert_id(pdo_dbh_t *dbh, const char *name,
                                          size_t *len) /* {{{ */
{
    PDO_LOG_ENTER("pdo_snowflake_last_insert_id");
    /* NOT SUPPORTED */
    PDO_LOG_RETURN(0);
}
/* }}} */

/**
 * Turn an unquoted string into a quoted string for use in a query.
 *
 * Not implemented as Snowflake supports server side binding.
 *
 * @param dbh Pointer to the database handle initialized by the handle factory
 * @param unquoted Pointer to a character string containing the string to be quoted.
 * @param unquotedlen The length of the string to be quoted.
 * @param quoted Pointer to the address where a pointer to the newly quoted
 * string will be returned.
 * @param quotedlen The length of the new string.
 * @param paramtype A driver specific hint for driver that have alternate
 * quoting styles
 * @return 1 if success or 0 if error occurs
 */
static int snowflake_handle_quoter(pdo_dbh_t *dbh, const char *unquoted,
                                   size_t unquotedlen, char **quoted,
                                   size_t *quotedlen,
                                   enum pdo_param_type paramtype) /* {{{ */
{
    PDO_LOG_ENTER("snowflake_handle_quoter");
    /* NOT SUPPORTED */
    PDO_LOG_RETURN(0);
}
/* }}} */

/**
 * Begin a database transaction.
 * @param dbh Pointer to the database handle initialized by the handle factory
 * @return 1 if success or 0 if error occurs
 */
static int snowflake_handle_begin(pdo_dbh_t *dbh) /* {{{ */
{
    PDO_LOG_ENTER("snowflake_handle_begin");
    pdo_snowflake_db_handle *H = (pdo_snowflake_db_handle *) dbh->driver_data;
    SF_STATUS status = snowflake_trans_begin(H->server);
    int ret = status == SF_STATUS_SUCCESS ? 1 : 0;
    PDO_LOG_RETURN(ret);
}
/* }}} */

/**
 * Commit a database transaction.
 * @param dbh Pointer to the database handle initialized by the handle factory
 * @return 1 if success or 0 if error occurs
 */
static int snowflake_handle_commit(pdo_dbh_t *dbh) /* {{{ */
{
    PDO_LOG_ENTER("snowflake_handle_commit");
    pdo_snowflake_db_handle *H = (pdo_snowflake_db_handle *) dbh->driver_data;
    SF_STATUS status = snowflake_trans_commit(H->server);
    int ret = status == SF_STATUS_SUCCESS ? 1 : 0;
    PDO_LOG_RETURN(ret);
}
/* }}} */

/**
 * Rollback a database transaction.
 * @param dbh Pointer to the database handle initialized by the handle factory
 * @return 1 if success or 0 if error occurs
 */
static int snowflake_handle_rollback(pdo_dbh_t *dbh) /* {{{ */
{
    PDO_LOG_ENTER("snowflake_handle_rollback");
    pdo_snowflake_db_handle *H = (pdo_snowflake_db_handle *) dbh->driver_data;
    SF_STATUS status = snowflake_trans_rollback(H->server);
    int ret = status == SF_STATUS_SUCCESS ? 1 : 0;
    PDO_LOG_RETURN(ret);
}

/* }}} */

/* TODO: is this really used? */
static inline int snowflake_handle_autocommit(pdo_dbh_t *dbh) /* {{{ */
{
    PDO_LOG_ENTER("snowflake_handle_autocommit");
    PDO_LOG_RETURN(1);
}

/* }}} */

static int
pdo_snowflake_set_attribute(pdo_dbh_t *dbh, zend_long attr, zval *val) /* {{{ */
{
    zend_long lval = zval_get_long(val);
    zend_bool bval = lval ? (zend_bool) 1 : (zend_bool) 0;
    PDO_LOG_ENTER("pdo_snowflake_set_attribute");
    PDO_LOG_DBG("dbh=%p, attr=%l", dbh, attr);
    switch (attr) {
        case PDO_ATTR_AUTOCOMMIT:
            /* ignore if the new value equals the old one */
            if (dbh->auto_commit ^ bval) {
                dbh->auto_commit = bval;
                PDO_LOG_DBG(
                    "value=%s",
                    bval ? SF_BOOLEAN_INTERNAL_TRUE_STR
                         : SF_BOOLEAN_INTERNAL_FALSE_STR);
            }
            PDO_LOG_RETURN(1);
            break;
        default:
            PDO_LOG_DBG("unsupported attribute: %ld", attr);
            /* invalid attribute */
            PDO_LOG_RETURN(0);
            break;
    }
}

/* }}} */

static int
pdo_snowflake_get_attribute(pdo_dbh_t *dbh, zend_long attr,
                            zval *return_value) {
    pdo_snowflake_db_handle *H = (pdo_snowflake_db_handle *) dbh->driver_data;

    PDO_LOG_ENTER("pdo_snowflake_get_attribute");
    PDO_LOG_DBG("dbh=%p", dbh);
    PDO_LOG_DBG("attr=%l", attr);
    switch (attr) {
        /* TODO: add more attributes */
        case PDO_ATTR_AUTOCOMMIT:
            ZVAL_LONG(return_value, dbh->auto_commit);
            PDO_LOG_RETURN(1);
            break;
        case PDO_ATTR_CLIENT_VERSION:
            ZVAL_STRINGL(return_value, PDO_SNOWFLAKE_VERSION, strlen(PDO_SNOWFLAKE_VERSION));
            PDO_LOG_RETURN(1);
            break;
        case PDO_SNOWFLAKE_ATTR_QUERY_ID:
            ZVAL_STRINGL(return_value, H->last_qid, strlen(H->last_qid));
            PDO_LOG_RETURN(1);
            break;
        default:
            /**/
            PDO_LOG_RETURN(0);
    }
    PDO_LOG_RETURN(0);
}
/* }}} */

/**
 * Test whether or not a persistent connection to a database is alive and ready
 * for use.
 *
 * This function returns 1 if the database connection is alive and ready for
 * use, otherwise it should return 0 to indicate failure or lack of support.
 *
 * @param dbh Pointer to the database handle initialized by the handle factory
 * @return 1 if success or 0 if error occurs
 */
static int pdo_snowflake_check_liveness(pdo_dbh_t *dbh) /* {{{ */
{
    PDO_LOG_ENTER("pdo_snowflake_check_liveness");
    /* TODO: this can run just run select 1 and check the response */
    PDO_LOG_RETURN(0);
}
/* }}} */

/* {{{ snowflake_methods */
#if (PHP_VERSION_ID < 80100)
static struct pdo_dbh_methods snowflake_methods = {
    snowflake_handle_closer,
    snowflake_handle_preparer,
    snowflake_handle_doer,
    snowflake_handle_quoter,
    snowflake_handle_begin,
    snowflake_handle_commit,
    snowflake_handle_rollback,
    pdo_snowflake_set_attribute,
    pdo_snowflake_last_insert_id,
    pdo_snowflake_fetch_error_func,
    pdo_snowflake_get_attribute,
    pdo_snowflake_check_liveness,
    NULL, /* get_driver_methods */
    NULL, /* persistent_shutdown */
    NULL /* in_transaction*/
};
#else
static void snowflake_handle_closer_newif(pdo_dbh_t *dbh)
{
    snowflake_handle_closer(dbh);
}
static bool snowflake_handle_preparer_newif(pdo_dbh_t *dbh, zend_string *sql, pdo_stmt_t *stmt, zval *driver_options)
{
    return (snowflake_handle_preparer(dbh, ZSTR_VAL(sql), ZSTR_LEN(sql), stmt, driver_options) != 0);
}
static zend_long snowflake_handle_doer_newif(pdo_dbh_t *dbh, const zend_string *sql)
{
    return snowflake_handle_doer(dbh, ZSTR_VAL(sql), ZSTR_LEN(sql));
}
static zend_string* snowflake_handle_quoter_newif(pdo_dbh_t *dbh, const zend_string *unquoted, enum pdo_param_type paramtype)
{
    PDO_LOG_ENTER("snowflake_handle_quoter");
    /* NOT SUPPORTED */
    PDO_LOG_RETURN(0);
}
static bool snowflake_handle_begin_newif(pdo_dbh_t *dbh)
{
    return (snowflake_handle_begin(dbh) != 0);
}
static bool snowflake_handle_commit_newif(pdo_dbh_t *dbh)
{
    return (snowflake_handle_commit(dbh) != 0);
}
static bool snowflake_handle_rollback_newif(pdo_dbh_t *dbh)
{
    return (snowflake_handle_rollback(dbh) != 0);
}
static bool pdo_snowflake_set_attribute_newif(pdo_dbh_t *dbh, zend_long attr, zval *val)
{
    return (pdo_snowflake_set_attribute(dbh, attr, val) != 0);
}
static zend_string * pdo_snowflake_last_insert_id_newif(pdo_dbh_t *dbh, const zend_string *name)
{
    PDO_LOG_ENTER("pdo_snowflake_last_insert_id");
    /* NOT SUPPORTED */
    PDO_LOG_RETURN(0);
}
static zend_result pdo_snowflake_check_liveness_newif(pdo_dbh_t *dbh)
{
    return (pdo_snowflake_check_liveness(dbh) == 0) ? FAILURE : SUCCESS;
}
static void pdo_snowflake_fetch_error_func_newif(pdo_dbh_t *dbh, pdo_stmt_t *stmt, zval *info)
{
    pdo_snowflake_fetch_error_func(dbh, stmt, info);
}
static struct pdo_dbh_methods snowflake_methods = {
    snowflake_handle_closer_newif,
    snowflake_handle_preparer_newif,
    snowflake_handle_doer_newif,
    snowflake_handle_quoter_newif,
    snowflake_handle_begin_newif,
    snowflake_handle_commit_newif,
    snowflake_handle_rollback_newif,
    pdo_snowflake_set_attribute_newif,
    pdo_snowflake_last_insert_id_newif,
    pdo_snowflake_fetch_error_func_newif,
    pdo_snowflake_get_attribute,
    pdo_snowflake_check_liveness_newif,
    NULL, /* get_driver_methods */
    NULL, /* persistent_shutdown */
    NULL /* in_transaction*/
};
#endif
/* }}} */

/**
 * Create a database handle. For most databases this involves establishing a
 * connection to the database. In some cases, a persistent connection may be
 * requested, in other cases connection pooling may be requested. All of these
 * are database/driver dependent.
 *
 * @param dbh Pointer to the database handle initialized by the handle factory
 * @param driver_options An array of driver options, keyed by integer
 * option number. See Database and Statement Attributes Table for a list of
 * possible attributes.
 * @return 1 if success or 0 if error occurs
 */
static int
pdo_snowflake_handle_factory(pdo_dbh_t *dbh, zval *driver_options) /* {{{ */
{
    PDO_LOG_ENTER("pdo_snowflake_handle_factory");
    pdo_snowflake_db_handle *H;
    size_t i;
    int ret = 0;
    int64 int_attr_value = 0;
    int8 int8_attr_value = 0;
    /* NOTE: the parameters are referenced by index, so if you change
     * the order of parameters, ensure changing the index of vars
     * in php_pdo_snowflake_int.h
     */
    struct pdo_data_src_parser vars[] = {
        {"host",                NULL,         0},
        {"port",                "443",        0},
        {"account",             NULL,         0},
        {"region",              NULL,         0},
        {"database",            NULL,         0},
        {"schema",              NULL,         0},
        {"warehouse",           NULL,         0},
        {"role",                NULL,         0},
        {"protocol",            "https",      0},
        {"insecure_mode",       NULL,         0},
        {"timezone",            NULL,         0},
        {"application",         NULL,         0},
        {"authenticator",       NULL,         0},
        {"priv_key_file",       NULL,         0},
        {"priv_key_file_pwd",   NULL,         0},
        {"proxy",               NULL,         0},
        {"no_proxy",            NULL,         0},
        {"disablequerycontextcache", "false", 0},
        {"includeretryreason",  "true",       0},
        {"logintimeout",        "300",        0},
        {"maxhttpretries",      "7",          0},
        {"retrytimeout",        "300",        0},
        {"ocspfailopen",        "true",       0},
        {"disableocspchecks",   "false",      0},
        {"passcode",            NULL,         0},
        {"passcodeinpassword",  "false",      0},
        {"disablesamlurlcheck", "false",      0},
        {"crl_check",           "false",      0},
        {"crl_advisory",        "false",      0},
        {"crl_allow_no_crl",    "false",      0},
        {"crl_memory_caching",  "true",       0},
        {"crl_disk_caching",    "true",       0},
        {"crl_download_timeout", "120",       0},
        {"oauth_token_endpoint", NULL,      0},
        {"oauth_authorization_endpoint", NULL, 0},
        {"oauth_redirect_uri",  NULL,         0},
        {"oauth_client_id",     NULL,         0},
        {"oauth_client_secret", NULL,         0},
        {"oauth_scope",   NULL,         0},
        {"single_use_refresh_token", "false", 0},
#ifdef __LINUX__
        {"client_store_temporary_credential", "false", 0},
        {"client_request_mfa_token", "false", 0}
#else
        {"client_store_temporary_credential", "true", 0},
        {"client_request_mfa_token", "true", 0}
#endif
    };

    // Parse the input data parameters
    php_pdo_parse_data_source(dbh->data_source, dbh->data_source_len, vars,
                              sizeof(vars) /
                              sizeof(struct pdo_data_src_parser));

    H = pecalloc(1, sizeof(pdo_snowflake_db_handle), dbh->is_persistent);
    H->last_qid[0] = '\0';

    //TODO set error stuff

    /* allocate an environment */

    /* handle for the server */
    if (!(H->server = snowflake_init())) {
        pdo_snowflake_error(dbh);
        goto cleanup;
    }

    dbh->driver_data = H;

    if (driver_options) {
        //TODO Set other non-essential parameters, i.e. timeout, emulate, etc.
        zend_string *ca_bundle_file = pdo_attr_strval(
            driver_options, PDO_SNOWFLAKE_ATTR_SSL_CAPATH, NULL);
        // TODO create function to set SSL Version
        zend_long ssl_version = pdo_attr_lval(
            driver_options, PDO_SNOWFLAKE_ATTR_SSL_VERSION, -1);
        zend_long disable_verify_peer = pdo_attr_lval(
            driver_options,
            PDO_SNOWFLAKE_ATTR_SSL_VERIFY_CERTIFICATE_REVOCATION_STATUS, 1) ? 0
                                                                            : 1;
        /* auto commit */
        zend_long auto_commit = pdo_attr_lval(
            driver_options,
            PDO_ATTR_AUTOCOMMIT, 1);

        /*TODO: disable verify peer? do we need this option? */
        snowflake_global_set_attribute(
            SF_GLOBAL_DISABLE_VERIFY_PEER, &disable_verify_peer);

        if (ssl_version != -1) {
            /* TODO: not allowed older than TLS 1.2 */
            snowflake_global_set_attribute(SF_GLOBAL_SSL_VERSION, &ssl_version);
        }
        if (ca_bundle_file) {
            zend_string_release(ca_bundle_file);
        }

        /* autocommit */
        dbh->auto_commit = (unsigned) auto_commit;
    }

    PDO_LOG_INF("Snowflake PHP PDO Driver: %s", PDO_SNOWFLAKE_VERSION);

    snowflake_set_attribute(H->server, SF_CON_APPLICATION_NAME,
                            PHP_PDO_SNOWFLAKE_NAME);
    snowflake_set_attribute(H->server, SF_CON_APPLICATION_VERSION, PDO_SNOWFLAKE_VERSION);
    snowflake_set_attribute(H->server, SF_CON_USER, dbh->username);
    PDO_LOG_DBG(
        "user: %s", dbh->username);
    snowflake_set_attribute(H->server, SF_CON_PASSWORD, dbh->password);
    PDO_LOG_DBG(
        "password: %s", dbh->password != NULL ? "******" : "(NULL)");
    snowflake_set_attribute(
        H->server, SF_CON_HOST, vars[PDO_SNOWFLAKE_CONN_ATTR_HOST_IDX].optval);
    PDO_LOG_DBG(
        "host: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_HOST_IDX].optval);
    snowflake_set_attribute(
        H->server, SF_CON_PORT, vars[PDO_SNOWFLAKE_CONN_ATTR_PORT_IDX].optval);
    PDO_LOG_DBG(
        "port: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_PORT_IDX].optval);
    snowflake_set_attribute(
        H->server, SF_CON_ACCOUNT,
        vars[PDO_SNOWFLAKE_CONN_ATTR_ACCOUNT_IDX].optval);
    PDO_LOG_DBG(
        "account: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_ACCOUNT_IDX].optval);
    snowflake_set_attribute(
        H->server, SF_CON_REGION,
        vars[PDO_SNOWFLAKE_CONN_ATTR_REGION_IDX].optval);
    PDO_LOG_DBG(
        "region: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_REGION_IDX].optval);
    snowflake_set_attribute(
        H->server, SF_CON_DATABASE,
        vars[PDO_SNOWFLAKE_CONN_ATTR_DATABASE_IDX].optval);
    PDO_LOG_DBG(
        "database: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_DATABASE_IDX].optval);
    snowflake_set_attribute(
        H->server, SF_CON_SCHEMA,
        vars[PDO_SNOWFLAKE_CONN_ATTR_SCHEMA_IDX].optval);
    PDO_LOG_DBG(
        "schema: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_SCHEMA_IDX].optval);
    snowflake_set_attribute(
        H->server, SF_CON_WAREHOUSE,
        vars[PDO_SNOWFLAKE_CONN_ATTR_WAREHOUSE_IDX].optval);
    PDO_LOG_DBG(
        "warehouse: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_WAREHOUSE_IDX].optval);
    snowflake_set_attribute(
        H->server, SF_CON_ROLE, vars[PDO_SNOWFLAKE_CONN_ATTR_ROLE_IDX].optval);
    PDO_LOG_DBG(
        "role: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_ROLE_IDX].optval);
    snowflake_set_attribute(
        H->server, SF_CON_PROTOCOL,
        vars[PDO_SNOWFLAKE_CONN_ATTR_PROTOCOL_IDX].optval);
    PDO_LOG_DBG(
        "protocol: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_PROTOCOL_IDX].optval);
    snowflake_set_attribute(
        H->server, SF_CON_INSECURE_MODE,
        vars[PDO_SNOWFLAKE_CONN_ATTR_INSECURE_MODE_IDX].optval);
    PDO_LOG_DBG(
        "insecureMode: %s",
        vars[PDO_SNOWFLAKE_CONN_ATTR_INSECURE_MODE_IDX].optval);
    snowflake_set_attribute(
        H->server, SF_CON_AUTOCOMMIT,
        dbh->auto_commit ? &SF_BOOLEAN_TRUE : &SF_BOOLEAN_FALSE);

    if (vars[PDO_SNOWFLAKE_CONN_ATTR_TIMEZONE_IDX].optval != NULL) {
        /* timezone */
        snowflake_set_attribute(
            H->server, SF_CON_TIMEZONE,
            vars[PDO_SNOWFLAKE_CONN_ATTR_TIMEZONE_IDX].optval);
    }
    PDO_LOG_DBG(
        "timezone: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_TIMEZONE_IDX].optval);
    PDO_LOG_DBG(
        "autocommit: %u", dbh->auto_commit);

    if (vars[PDO_SNOWFLAKE_CONN_ATTR_APPLICATION_IDX].optval != NULL) {
        /* applicaiton */
        snowflake_set_attribute(
            H->server, SF_CON_APPLICATION,
            vars[PDO_SNOWFLAKE_CONN_ATTR_APPLICATION_IDX].optval);
    }
    PDO_LOG_DBG(
        "application: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_APPLICATION_IDX].optval);

    if (vars[PDO_SNOWFLAKE_CONN_ATTR_AUTHENTICATOR_IDX].optval != NULL) {
        /* authenticator */
        snowflake_set_attribute(
            H->server, SF_CON_AUTHENTICATOR,
            vars[PDO_SNOWFLAKE_CONN_ATTR_AUTHENTICATOR_IDX].optval);
    }
    PDO_LOG_DBG(
        "authenticator: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_AUTHENTICATOR_IDX].optval);

    if (vars[PDO_SNOWFLAKE_CONN_ATTR_PRIV_KEY_FILE_IDX].optval != NULL) {
        /* priv_key_file */
        snowflake_set_attribute(
            H->server, SF_CON_PRIV_KEY_FILE,
            vars[PDO_SNOWFLAKE_CONN_ATTR_PRIV_KEY_FILE_IDX].optval);
    }
    PDO_LOG_DBG(
        "priv_key_file: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_PRIV_KEY_FILE_IDX].optval);

    if (vars[PDO_SNOWFLAKE_CONN_ATTR_PRIV_KEY_FILE_PWD_IDX].optval != NULL) {
        /* priv_key_file_pwd */
        snowflake_set_attribute(
            H->server, SF_CON_PRIV_KEY_FILE_PWD,
            vars[PDO_SNOWFLAKE_CONN_ATTR_PRIV_KEY_FILE_PWD_IDX].optval);
    }
    PDO_LOG_DBG(
        "priv_key_file_pwd: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_PRIV_KEY_FILE_PWD_IDX].optval);

    if (vars[PDO_SNOWFLAKE_CONN_ATTR_PROXY_IDX].optval != NULL) {
        /* proxy */
        snowflake_set_attribute(
            H->server, SF_CON_PROXY,
            vars[PDO_SNOWFLAKE_CONN_ATTR_PROXY_IDX].optval);
    }
    PDO_LOG_DBG(
        "proxy: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_PROXY_IDX].optval != NULL ? "******" : "(NULL)");

    if (vars[PDO_SNOWFLAKE_CONN_ATTR_NO_PROXY_IDX].optval != NULL) {
        /* no_proxy */
        snowflake_set_attribute(
            H->server, SF_CON_NO_PROXY,
            vars[PDO_SNOWFLAKE_CONN_ATTR_NO_PROXY_IDX].optval);
    }
    PDO_LOG_DBG(
        "no_proxy: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_NO_PROXY_IDX].optval);

    snowflake_set_attribute(
        H->server, SF_CON_DISABLE_QUERY_CONTEXT_CACHE,
        (strcasecmp(vars[PDO_SNOWFLAKE_CONN_ATTR_DISABLE_QUERY_CONTEXT_CACHE_IDX].optval, "true") == 0) ?
            &SF_BOOLEAN_TRUE : &SF_BOOLEAN_FALSE);
    PDO_LOG_DBG(
        "disablequerycontextcache: %s",
        vars[PDO_SNOWFLAKE_CONN_ATTR_DISABLE_QUERY_CONTEXT_CACHE_IDX].optval);

    snowflake_set_attribute(
        H->server, SF_CON_INCLUDE_RETRY_REASON,
        (strcasecmp(vars[PDO_SNOWFLAKE_CONN_ATTR_INCLUDE_RETRY_REASON_IDX].optval, "true") == 0) ?
            &SF_BOOLEAN_TRUE : &SF_BOOLEAN_FALSE);
    PDO_LOG_DBG(
        "includeretryreason: %s",
        vars[PDO_SNOWFLAKE_CONN_ATTR_INCLUDE_RETRY_REASON_IDX].optval);

    if (vars[PDO_SNOWFLAKE_CONN_ATTR_LOGIN_TIMEOUT_IDX].optval != NULL) {
        int_attr_value = strtoll(vars[PDO_SNOWFLAKE_CONN_ATTR_LOGIN_TIMEOUT_IDX].optval, NULL, 10);
        snowflake_set_attribute(
            H->server, SF_CON_LOGIN_TIMEOUT,
            &int_attr_value);
        PDO_LOG_DBG(
            "logintimeout: %d", int_attr_value);
    }

    if (vars[PDO_SNOWFLAKE_CONN_ATTR_MAX_RETRIES_IDX].optval != NULL) {
        int8_attr_value = strtol(vars[PDO_SNOWFLAKE_CONN_ATTR_MAX_RETRIES_IDX].optval, NULL, 10);
        snowflake_set_attribute(
            H->server, SF_CON_MAX_RETRY,
            &int8_attr_value);
        PDO_LOG_DBG(
            "maxhttpretries: %d", int8_attr_value);
    }

    if (vars[PDO_SNOWFLAKE_CONN_ATTR_RETRY_TIMEOUT_IDX].optval != NULL) {
        int_attr_value = strtoll(vars[PDO_SNOWFLAKE_CONN_ATTR_RETRY_TIMEOUT_IDX].optval, NULL, 10);
        snowflake_set_attribute(
            H->server, SF_CON_RETRY_TIMEOUT,
            &int_attr_value);
        PDO_LOG_DBG(
            "retryimeout: %d", int_attr_value);
    }

    snowflake_set_attribute(
        H->server, SF_CON_OCSP_FAIL_OPEN,
        (strcasecmp(vars[PDO_SNOWFLAKE_CONN_ATTR_OCSP_FAIL_OPEN_IDX].optval, "true") == 0) ?
            &SF_BOOLEAN_TRUE : &SF_BOOLEAN_FALSE);
    PDO_LOG_DBG(
        "ocspfailopen: %s",
        vars[PDO_SNOWFLAKE_CONN_ATTR_OCSP_FAIL_OPEN_IDX].optval);

    snowflake_global_set_attribute(SF_GLOBAL_OCSP_CHECK,
        (strcasecmp(vars[PDO_SNOWFLAKE_CONN_ATTR_OCSP_DISABLE_IDX].optval, "true") == 0) ?
            &SF_BOOLEAN_FALSE : &SF_BOOLEAN_TRUE);
    PDO_LOG_DBG(
        "disableocspchecks: %s",
        vars[PDO_SNOWFLAKE_CONN_ATTR_OCSP_DISABLE_IDX].optval);

    if (vars[PDO_SNOWFLAKE_CONN_ATTR_PASSCODE_IDX].optval != NULL) {
        /* passcode */
        snowflake_set_attribute(
            H->server, SF_CON_PASSCODE,
            vars[PDO_SNOWFLAKE_CONN_ATTR_PASSCODE_IDX].optval);
    }
    PDO_LOG_DBG(
        "passcode: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_PASSCODE_IDX].optval != NULL ? "******" : "(NULL)");

    snowflake_set_attribute(
        H->server, SF_CON_PASSCODE_IN_PASSWORD,
        (strcasecmp(vars[PDO_SNOWFLAKE_CONN_ATTR_PASSCODE_IN_PASSWORD_IDX].optval, "true") == 0) ?
            &SF_BOOLEAN_TRUE : &SF_BOOLEAN_FALSE);
    PDO_LOG_DBG(
        "passcodeinpassword: %s",
        vars[PDO_SNOWFLAKE_CONN_ATTR_PASSCODE_IN_PASSWORD_IDX].optval);

    snowflake_set_attribute(H->server, SF_CON_DISABLE_SAML_URL_CHECK, 
        (strcasecmp(vars[PDO_SNOWFLAKE_CONN_ATTR_DISABLE_SAML_URL_CHECK_IDX].optval, "true") == 0)? &SF_BOOLEAN_TRUE :  &SF_BOOLEAN_FALSE);
    PDO_LOG_DBG("disablesamlURLcheck: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_DISABLE_SAML_URL_CHECK_IDX].optval);

    snowflake_set_attribute(H->server, SF_CON_CRL_CHECK,
        (strcasecmp(vars[PDO_SNOWFLAKE_CONN_ATTR_CRL_CHECK_IDX].optval, "true") == 0)? &SF_BOOLEAN_TRUE :  &SF_BOOLEAN_FALSE);
    PDO_LOG_DBG("crl_check: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_DISABLE_SAML_URL_CHECK_IDX].optval);

    snowflake_set_attribute(H->server, SF_CON_CRL_ADVISORY,
        (strcasecmp(vars[PDO_SNOWFLAKE_CONN_ATTR_CRL_ADVISORY_IDX].optval, "true") == 0)? &SF_BOOLEAN_TRUE :  &SF_BOOLEAN_FALSE);
    PDO_LOG_DBG("crl_advisory: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_CRL_ADVISORY_IDX].optval);

    snowflake_set_attribute(H->server, SF_CON_CRL_ALLOW_NO_CRL,
        (strcasecmp(vars[PDO_SNOWFLAKE_CONN_ATTR_CRL_ALLOW_NO_CRL_IDX].optval, "true") == 0)? &SF_BOOLEAN_TRUE :  &SF_BOOLEAN_FALSE);
    PDO_LOG_DBG("crl_allow_no_crl: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_CRL_ALLOW_NO_CRL_IDX].optval);

    snowflake_set_attribute(H->server, SF_CON_CRL_MEMORY_CACHING,
        (strcasecmp(vars[PDO_SNOWFLAKE_CONN_ATTR_CRL_MEMORY_CACHING_IDX].optval, "true") == 0)? &SF_BOOLEAN_TRUE :  &SF_BOOLEAN_FALSE);
    PDO_LOG_DBG("crl_memory_caching: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_CRL_MEMORY_CACHING_IDX].optval);

    snowflake_set_attribute(H->server, SF_CON_CRL_DISK_CACHING,
        (strcasecmp(vars[PDO_SNOWFLAKE_CONN_ATTR_CRL_DISK_CACHING_IDX].optval, "true") == 0)? &SF_BOOLEAN_TRUE :  &SF_BOOLEAN_FALSE);
    PDO_LOG_DBG("crl_disk_caching: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_CRL_DISK_CACHING_IDX].optval);

    if (vars[PDO_SNOWFLAKE_CONN_ATTR_CRL_DOWNLOAD_TIMEOUT_IDX].optval != NULL) {
        int_attr_value = strtoll(vars[PDO_SNOWFLAKE_CONN_ATTR_CRL_DOWNLOAD_TIMEOUT_IDX].optval, NULL, 10);
        snowflake_set_attribute(
            H->server, SF_CON_CRL_DOWNLOAD_TIMEOUT,
            &int_attr_value);
        PDO_LOG_DBG(
            "crl_download_timeout: %d", int_attr_value);
    }

    snowflake_set_attribute(H->server, SF_CON_OAUTH_AUTHORIZATION_ENDPOINT,
        vars[PDO_SNOWFLAKE_CONN_ATTR_OAUTH_AUTHORIZATION_ENDPOINT].optval);
    PDO_LOG_DBG("oauth_authorization_endpoint: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_OAUTH_AUTHORIZATION_ENDPOINT].optval);

    snowflake_set_attribute(H->server, SF_CON_OAUTH_TOKEN_ENDPOINT,
        vars[PDO_SNOWFLAKE_CONN_ATTR_OAUTH_TOKEN_ENDPOINT].optval);
    PDO_LOG_DBG("oauth_token_endpoint: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_OAUTH_TOKEN_ENDPOINT].optval);

    snowflake_set_attribute(H->server, SF_CON_OAUTH_REDIRECT_URI,
        vars[PDO_SNOWFLAKE_CONN_ATTR_OAUTH_REDIRECT_URI].optval);
    PDO_LOG_DBG("oauth_redirect_url: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_OAUTH_REDIRECT_URI].optval);

    snowflake_set_attribute(H->server, SF_CON_OAUTH_CLIENT_ID,
        vars[PDO_SNOWFLAKE_CONN_ATTR_OAUTH_CLIENT_ID].optval);
    PDO_LOG_DBG("oauth_client_id: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_OAUTH_CLIENT_ID].optval);
    
    snowflake_set_attribute(H->server, SF_CON_OAUTH_CLIENT_SECRET,
        vars[PDO_SNOWFLAKE_CONN_ATTR_OAUTH_CLIENT_SECRET].optval);
    PDO_LOG_DBG("oauth_client_secret: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_OAUTH_CLIENT_SECRET].optval!= NULL ? "******" : "(NULL)");

    snowflake_set_attribute(H->server, SF_CON_OAUTH_SCOPE,
        vars[PDO_SNOWFLAKE_CONN_ATTR_OAUTH_SCOPE].optval);
    PDO_LOG_DBG("oauth_scope: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_OAUTH_SCOPE].optval);

    snowflake_set_attribute(H->server, SF_CON_SINGLE_USE_REFRESH_TOKEN,
        (strcasecmp(vars[PDO_SNOWFLAKE_CONN_ATTR_SINGLE_USE_REFRESH_TOKEN].optval, "true") == 0)? &SF_BOOLEAN_TRUE :  &SF_BOOLEAN_FALSE);
    PDO_LOG_DBG("single_use_refresh_token: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_SINGLE_USE_REFRESH_TOKEN].optval);

    snowflake_set_attribute(H->server, SF_CON_CLIENT_STORE_TEMPORARY_CREDENTIAL,
        (strcasecmp(vars[PDO_SNOWFLAKE_CONN_ATTR_CLIENT_STORE_TEMPORARY_CREDENTIAL].optval, "true") == 0)? &SF_BOOLEAN_TRUE :  &SF_BOOLEAN_FALSE);
    PDO_LOG_DBG("client_store_temporary_credential: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_CLIENT_STORE_TEMPORARY_CREDENTIAL].optval);

    snowflake_set_attribute(H->server, SF_CON_CLIENT_REQUEST_MFA_TOKEN,
        (strcasecmp(vars[PDO_SNOWFLAKE_CONN_ATTR_CLIENT_REQUEST_MFA_TOKEN].optval, "true") == 0)? &SF_BOOLEAN_TRUE :  &SF_BOOLEAN_FALSE);
    PDO_LOG_DBG("client_request_mfa_token: %s", vars[PDO_SNOWFLAKE_CONN_ATTR_CLIENT_REQUEST_MFA_TOKEN].optval);

    int8 ocsp_enabled = (strcasecmp(vars[PDO_SNOWFLAKE_CONN_ATTR_OCSP_DISABLE_IDX].optval, "true") != 0);
    int8 crl_enabled = (strcasecmp(vars[PDO_SNOWFLAKE_CONN_ATTR_CRL_CHECK_IDX].optval, "true") == 0);
    
    if (ocsp_enabled && crl_enabled) {
        PDO_LOG_ERR("Both OCSP and CRL checks are enabled. Only one revocation check method can be enabled at a time.");
        
        strcpy(dbh->error_code, "HY000");
        zend_throw_exception_ex(
            php_pdo_get_exception(),
            1,
            "SQLSTATE[HY000] [1] Both host certificate revocation check methods (OCSP and CRL) are enabled. "
            "Please turn off crl_check or toggle OCSP with disableocspchecks.");
        ret = 0;
        goto cleanup;
    }

    if (snowflake_connect(H->server) > 0) {
        pdo_snowflake_error(dbh);
        goto cleanup;
    }
    ret = 1;

cleanup:
    for (i = 0; i < sizeof(vars) / sizeof(vars[0]); i++) {
        if (vars[i].freeme) {
            efree(vars[i].optval);
        }
    }

    dbh->methods = &snowflake_methods;

    PDO_LOG_RETURN(ret);
}

/* }}} */


pdo_driver_t pdo_snowflake_driver = {
    PDO_DRIVER_HEADER(snowflake),
    pdo_snowflake_handle_factory
};
