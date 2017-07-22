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

static int pdo_snowflake_stmt_dtor(pdo_stmt_t *stmt) /* {{{ */
{
    PDO_DBG_RETURN(1);
}
/* }}} */

static int pdo_snowflake_stmt_execute(pdo_stmt_t *stmt) /* {{{ */
{
    PDO_DBG_RETURN(1);
}
/* }}} */

static int pdo_snowflake_stmt_fetch(pdo_stmt_t *stmt, enum pdo_fetch_orientation ori, zend_long offset) /* {{{ */
{
    PDO_DBG_RETURN(1);
}
/* }}} */

static int pdo_snowflake_stmt_describe(pdo_stmt_t *stmt, int colno) /* {{{ */
{
    PDO_DBG_RETURN(1);
}
/* }}} */

static int pdo_snowflake_stmt_get_col(pdo_stmt_t *stmt, int colno, char **ptr, size_t *len, int *caller_frees) /* {{{ */
{
    PDO_DBG_RETURN(1);
}
/* }}} */

static int pdo_snowflake_stmt_param_hook(pdo_stmt_t *stmt, struct pdo_bound_param_data *param, enum pdo_param_event event_type) /* {{{ */
{
    PDO_DBG_RETURN(1);
}
/* }}} */

static int pdo_snowflake_stmt_col_meta(pdo_stmt_t *stmt, zend_long colno, zval *return_value) /* {{{ */
{
    PDO_DBG_RETURN(1);
}
/* }}} */

static int pdo_snowflake_stmt_next_rowset(pdo_stmt_t *stmt) /* {{{ */
{
    PDO_DBG_RETURN(1);
}
/* }}} */

static int pdo_snowflake_stmt_cursor_closer(pdo_stmt_t *stmt) /* {{{ */
{
    PDO_DBG_RETURN(1);
}
/* }}} */

struct pdo_stmt_methods snowflake_stmt_methods = {
	pdo_snowflake_stmt_dtor,
	pdo_snowflake_stmt_execute,
	pdo_snowflake_stmt_fetch,
	pdo_snowflake_stmt_describe,
	pdo_snowflake_stmt_get_col,
	pdo_snowflake_stmt_param_hook,
	NULL, /* set_attr */
	NULL, /* get_attr */
	pdo_snowflake_stmt_col_meta,
	pdo_snowflake_stmt_next_rowset,
	pdo_snowflake_stmt_cursor_closer
};
