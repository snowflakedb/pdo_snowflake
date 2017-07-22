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

int _pdo_snowflake_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, const char *file, int line) /* {{{ */
{
    return PDO_DBG_RETURN(1);
}
/* }}} */

static int pdo_snowflake_fetch_error_func(pdo_dbh_t *dbh, pdo_stmt_t *stmt, zval *info) /* {{{ */
{
    return PDO_DBG_RETURN(1);
}
/* }}} */

static int snowflake_handle_closer(pdo_dbh_t *dbh) /* {{{ */
{
    return PDO_DBG_RETURN(1);
}
/* }}} */

static int snowflake_handle_preparer(pdo_dbh_t *dbh, const char *sql, size_t sql_len, pdo_stmt_t *stmt, zval *driver_options) /* {{{ */
{
    return PDO_DBG_RETURN(1);
}
/* }}} */

static zend_long snowflake_handle_doer(pdo_dbh_t *dbh, const char *sql, size_t sql_len) /* {{{ */
{
    return PDO_DBG_RETURN(1);
}
/* }}} */

static char *pdo_snowflake_last_insert_id(pdo_dbh_t *dbh, const char *name, size_t *len) /* {{{ */
{
    return NULL;
}
/* }}} */

static int snowflake_handle_quoter(pdo_dbh_t *dbh, const char *unquoted, size_t unquotedlen, char **quoted, size_t *quotedlen, enum pdo_param_type paramtype ) /* {{{ */
{
    return PDO_DBG_RETURN(1);
}
/* }}} */

static int snowflake_handle_begin(pdo_dbh_t *dbh) /* {{{ */
{
    return PDO_DBG_RETURN(1);
}
/* }}} */

static int snowflake_handle_commit(pdo_dbh_t *dbh) /* {{{ */
{
    return PDO_DBG_RETURN(1);
}
/* }}} */

static int snowflake_handle_rollback(pdo_dbh_t *dbh) /* {{{ */
{
    return PDO_DBG_RETURN(1);
}
/* }}} */

static inline int snowflake_handle_autocommit(pdo_dbh_t *dbh) /* {{{ */
{
    return PDO_DBG_RETURN(1);
}
/* }}} */

static int pdo_snowflake_set_attribute(pdo_dbh_t *dbh, zend_long attr, zval *val) /* {{{ */
{
    return PDO_DBG_RETURN(1);
}
/* }}} */

static int pdo_snowflake_get_attribute(pdo_dbh_t *dbh, zend_long attr, zval *return_value)
{
    return PDO_DBG_RETURN(1);
}
/* }}} */

static int pdo_snowflake_check_liveness(pdo_dbh_t *dbh) /* {{{ */
{
    return PDO_DBG_RETURN(1);
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

static int pdo_snowflake_handle_factory(pdo_dbh_t *dbh, zval *driver_options) /* {{{ */
{
	PDO_DBG_RETURN(1);
}

pdo_driver_t pdo_snowflake_driver = {
	PDO_DRIVER_HEADER(snowflake),
	pdo_snowflake_handle_factory
};
