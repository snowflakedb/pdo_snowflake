/* Copyright (c) 2017 Snowflake Computing Inc. All right reserved.  */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "pdo/php_pdo.h"
#include "pdo/php_pdo_driver.h"
#include "php_pdo_snowflake.h"
#include "php_pdo_snowflake_int.h"
#include "zend_exceptions.h"
#include <snowflake_client.h>

int _pdo_snowflake_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, const char *file, int line) /* {{{ */
{
    PDO_DBG_RETURN(1);
}
/* }}} */

static int pdo_snowflake_fetch_error_func(pdo_dbh_t *dbh, pdo_stmt_t *stmt, zval *info) /* {{{ */
{
    PDO_DBG_RETURN(1);
}
/* }}} */

/**
 * Close an open database.
 *
 * @param dbh Pointer to the database handle initialized by the handle factory
 * @return 1 if success or 0 if error occurs
 */
static int snowflake_handle_closer(pdo_dbh_t *dbh) /* {{{ */
{
    PDO_DBG_ENTER("snowflake_handle_closer");
    pdo_snowflake_db_handle *H = (pdo_snowflake_db_handle *)dbh->driver_data;

    if (H) {
        if (H->server) {
            snowflake_term(H->server);
            H->server = NULL;
        }
        pefree(H, dbh->is_persistent);
        dbh->driver_data = NULL;
    }
    snowflake_global_term(); // TODO: should be one time per process
    PDO_DBG_RETURN(1);
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
static int snowflake_handle_preparer(pdo_dbh_t *dbh, const char *sql, size_t sql_len, pdo_stmt_t *stmt, zval *driver_options) /* {{{ */
{
    PDO_DBG_ENTER("snowflake_handle_preparer");
    PDO_DBG_INF("dbh=%p", dbh);
    PDO_DBG_INF("sql=%.*s", (int)sql_len, sql);
    pdo_snowflake_db_handle *H = (pdo_snowflake_db_handle *)dbh->driver_data;
    pdo_snowflake_stmt *S = ecalloc(1, sizeof(pdo_snowflake_stmt));
    char *nsql = NULL;
    size_t nsql_len = 0;
    int ret;

    // TODO Add debugging info stuff

    S->H = H;
    stmt->driver_data = S;
    stmt->methods = &snowflake_stmt_methods;

    stmt->supports_placeholders = PDO_PLACEHOLDER_POSITIONAL;
    ret = pdo_parse_params(stmt, (char*)sql, sql_len, &nsql, &nsql_len);

    if (ret == 1) {
        /* query was rewritten */
        sql = nsql;
        sql_len = nsql_len;
    } else if (ret == -1) {
        /* failed to parse */
        strcpy(dbh->error_code, stmt->error_code);
        PDO_DBG_RETURN(0);
    }

    if (!(S->stmt = snowflake_stmt(H->server))) {
        pdo_snowflake_error(dbh);
        if (nsql) {
            efree(nsql);
        }
        PDO_DBG_RETURN(0);
    }

    if (snowflake_prepare(S->stmt, sql) != SF_STATUS_SUCCESS) {
        pdo_snowflake_error(dbh);
        if (nsql) {
            efree(nsql);
        }
        PDO_DBG_RETURN(0);
    }

    if (nsql) {
        efree(nsql);
    }
    dbh->alloc_own_columns = 1;

    PDO_DBG_RETURN(1);

    // TODO remove unnecessary goto statements

fallback:
end:
    stmt->supports_placeholders = PDO_PLACEHOLDER_NONE;

    PDO_DBG_RETURN(1);
}
/* }}} */

/**
 * Execute a raw SQL statement. No pdo_stmt_t is created.
 *
 * @param dbh Pointer to the database handle initialized by the handle factory
 * @param sql Pointer to a character string containing the SQL statement to be prepared.
 * @param sql_len The length of the SQL statement.
 * @return 1 if success or 0 if error occurs
 */
static zend_long snowflake_handle_doer(pdo_dbh_t *dbh, const char *sql, size_t sql_len) /* {{{ */
{
    PDO_DBG_ENTER("snowflake_handle_doer");
    int ret = 0;
    pdo_snowflake_db_handle *H = (pdo_snowflake_db_handle *)dbh->driver_data;

    // TODO add debugging statements

    SNOWFLAKE_STMT *sfstmt = snowflake_stmt(H->server);
    if (snowflake_query(sfstmt, sql) == SF_STATUS_SUCCESS) {
        int64 rows = snowflake_affected_rows(sfstmt);
        if (rows == -1) {
            // TODO copy error from sfstmt to snowflake db handle
            pdo_snowflake_error(dbh);
            ret = -1; // TODO add ternary expression like: H->einfo.errcode ? -1 : 0
            goto cleanup;
        }
        // return number of rows affected
        ret = (int) rows;
    } else {
        // TODO copy error from sfstmt to snowflake db handle
        pdo_snowflake_error(dbh);
        ret = -1;
        goto cleanup;
    }

cleanup:
    snowflake_stmt_close(sfstmt);

    PDO_DBG_RETURN(ret);
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
static char *pdo_snowflake_last_insert_id(pdo_dbh_t *dbh, const char *name, size_t *len) /* {{{ */
{
    PDO_DBG_ENTER("pdo_snowflake_last_insert_id");
    /* NOT SUPPORTED */
    PDO_DBG_RETURN(0);
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
static int snowflake_handle_quoter(pdo_dbh_t *dbh, const char *unquoted, size_t unquotedlen, char **quoted, size_t *quotedlen, enum pdo_param_type paramtype ) /* {{{ */
{
    PDO_DBG_ENTER("snowflake_handle_quoter");
    /* NOT SUPPORTED */
    PDO_DBG_RETURN(0);
}
/* }}} */

/**
 * Begin a database transaction.
 * @param dbh Pointer to the database handle initialized by the handle factory
 * @return 1 if success or 0 if error occurs
 */
static int snowflake_handle_begin(pdo_dbh_t *dbh) /* {{{ */
{
    PDO_DBG_ENTER("snowflake_handle_begin");
    PDO_DBG_RETURN(1);
}
/* }}} */

/**
 * Commit a database transaction.
 * @param dbh Pointer to the database handle initialized by the handle factory
 * @return 1 if success or 0 if error occurs
 */
static int snowflake_handle_commit(pdo_dbh_t *dbh) /* {{{ */
{
    PDO_DBG_ENTER("snowflake_handle_commit");
    PDO_DBG_RETURN(1);
}
/* }}} */

/**
 * Rollback a database transaction.
 * @param dbh Pointer to the database handle initialized by the handle factory
 * @return 1 if success or 0 if error occurs
 */
static int snowflake_handle_rollback(pdo_dbh_t *dbh) /* {{{ */
{
    PDO_DBG_ENTER("snowflake_handle_rollback");
    PDO_DBG_RETURN(1);
}
/* }}} */

static inline int snowflake_handle_autocommit(pdo_dbh_t *dbh) /* {{{ */
{
    PDO_DBG_ENTER("snowflake_handle_autocommit");
    PDO_DBG_RETURN(1);
}
/* }}} */

static int pdo_snowflake_set_attribute(pdo_dbh_t *dbh, zend_long attr, zval *val) /* {{{ */
{
    PDO_DBG_ENTER("pdo_snowflake_set_attribute");
    PDO_DBG_RETURN(1);
}
/* }}} */

static int pdo_snowflake_get_attribute(pdo_dbh_t *dbh, zend_long attr, zval *return_value)
{
    PDO_DBG_ENTER("pdo_snowflake_get_attribute");
    PDO_DBG_RETURN(1);
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
    PDO_DBG_ENTER("pdo_snowflake_check_liveness");
    /* TODO: this can run just run select 1 and check the response */
    PDO_DBG_RETURN(0);
}
/* }}} */

/* {{{ snowflake_methods */
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
	NULL,
	NULL,
	NULL
};
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
static int pdo_snowflake_handle_factory(pdo_dbh_t *dbh, zval *driver_options) /* {{{ */
{
    PDO_DBG_ENTER("pdo_snowflake_handle_factory");
    PDO_DBG_INF("dbh=%p", dbh);
	pdo_snowflake_db_handle *H;
	size_t i;
	int ret = 0;
	struct pdo_data_src_parser vars[] = {
			{"host","",0},
			{"port","",0 },
			{"account","",0 },
			{"database","",0 },
			{"schema","",0 },
            {"warehouse","", 0},
            {"role", "", 0},
            {"protocol", "https", 0},
            {"insecure_mode", "", 0}
	}; // 9 input parameters

    // Parse the input data parameters
    php_pdo_parse_data_source(dbh->data_source, dbh->data_source_len, vars, 9);

    // Run global init and allocate snowflake handle memory
    //TODO Come up with way to ensure global init is only called once
    snowflake_global_init();
    H = pecalloc(1, sizeof(pdo_snowflake_db_handle), dbh->is_persistent);

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
        zend_string *ca_bundle_file = pdo_attr_strval(driver_options, PDO_SNOWFLAKE_ATTR_SSL_CAPATH, NULL);
        // TODO create function to set SSL Version
        zend_long ssl_version = pdo_attr_lval(driver_options, PDO_SNOWFLAKE_ATTR_SSL_VERSION, -1);
        zend_long disable_verify_peer = pdo_attr_lval(driver_options, PDO_SNOWFLAKE_ATTR_SSL_VERIFY_CERTIFICATE_REVOCATION_STATUS, 1) ? 0 : 1;
        snowflake_global_set_attribute(SF_GLOBAL_DISABLE_VERIFY_PEER, &disable_verify_peer);
        snowflake_global_set_attribute(SF_GLOBAL_CA_BUNDLE_FILE, ca_bundle_file ? ZSTR_VAL(ca_bundle_file) : NULL);
        if (ssl_version != -1) {
            snowflake_global_set_attribute(SF_GLOBAL_SSL_VERSION, &ssl_version);
        }
        if (ca_bundle_file) {
            zend_string_release(ca_bundle_file);
        }
    }

    // Set context attributes
    snowflake_set_attr(H->server, SF_CON_USER, dbh->username);
    snowflake_set_attr(H->server, SF_CON_PASSWORD, dbh->password);
    snowflake_set_attr(H->server, SF_CON_HOST, vars[0].optval);
    snowflake_set_attr(H->server, SF_CON_PORT, vars[1].optval);
    snowflake_set_attr(H->server, SF_CON_ACCOUNT, vars[2].optval);
    snowflake_set_attr(H->server, SF_CON_DATABASE, vars[3].optval);
    snowflake_set_attr(H->server, SF_CON_SCHEMA, vars[4].optval);
    snowflake_set_attr(H->server, SF_CON_WAREHOUSE, vars[5].optval);
    snowflake_set_attr(H->server, SF_CON_ROLE, vars[6].optval);
    snowflake_set_attr(H->server, SF_CON_PROTOCOL, vars[7].optval);
    snowflake_set_attr(H->server, SF_CON_INSECURE_MODE, vars[8].optval);

    if (snowflake_connect(H->server) == SF_STATUS_ERROR) {
        pdo_snowflake_error(dbh);
        goto cleanup;
    }

    dbh->methods = &snowflake_methods;

    ret = 1;

cleanup:
    for (i = 0; i < sizeof(vars)/sizeof(vars[0]); i++) {
        if (vars[i].freeme) {
            efree(vars[i].optval);
        }
    }

    dbh->methods = &snowflake_methods;

	PDO_DBG_RETURN(ret);
}
/* }}} */


pdo_driver_t pdo_snowflake_driver = {
	PDO_DRIVER_HEADER(snowflake),
	pdo_snowflake_handle_factory
};
