/* Copyright (c) 2017 Snowflake Computing Inc. All right reserved.  */

#ifdef HAVE_CONFIG_H
#endif

#include "php.h"
#include "pdo/php_pdo_driver.h"
#include "php_pdo_snowflake_int.h"

/**
 * Mapping event enum to name
 */
static const char *const pdo_param_event_names[] =
  {
    "PDO_PARAM_EVT_ALLOC",
    "PDO_PARAM_EVT_FREE",
    "PDO_PARAM_EVT_EXEC_PRE",
    "PDO_PARAM_EVT_EXEC_POST",
    "PDO_PARAM_EVT_FETCH_PRE",
    "PDO_PARAM_EVT_FETCH_POST",
    "PDO_PARAM_EVT_NORMALIZE",
  };

static const char *const pdo_param_type_names[] = {
  "PDO_PARAM_NULL",
  "PDO_PARAM_INT",
  "PDO_PARAM_STR",
  "PDO_PARAM_LOB",
  "PDO_PARAM_STMT",
  "PDO_PARAM_BOOL",
  "PDO_PARAM_ZVAL",
  "PDO_PARAM_INPUT_OUTPUT",
};

static const char *const php_zval_type_names[] = {
  "IS_UNDEF",
  "IS_NULL",
  "IS_FALSE",
  "IS_TRUE",
  "IS_LONG",
  "IS_DOUBLE",
  "IS_STRING",
  "IS_ARRAY",
  "IS_OBJECT",
  "IS_RESOURCE",
  "IS_REFERENCE",
};

static void _pdo_snowflake_stmt_set_row_count(pdo_stmt_t *stmt) /* {{{ */
{
    zend_long row_count;
    pdo_snowflake_stmt *S = stmt->driver_data;

    PDO_DBG_ENTER("_pdo_snowflake_stmt_set_row_count");

    row_count = (zend_long) snowflake_affected_rows(S->stmt);
    PDO_DBG_INF("row count: %lld", row_count);
    if (row_count != (zend_long) -1) {
        stmt->row_count = row_count;
    }
}

/**
 * Destroy a previously constructed statement object.
 *
 * This should do whatever is necessary to free up any driver specific 
 * storage allocated for the statement. The return value from this function 
 * is ignored.
 * 
 * @param stmt Pointer to the statement structure initialized by handle_preparer.
 * @return 1 if success or 0 if error occurs
 */
static int pdo_snowflake_stmt_dtor(pdo_stmt_t *stmt) /* {{{ */
{
    PDO_DBG_ENTER("pdo_snowflake_stmt_dtor");
    pdo_snowflake_stmt *S = stmt->driver_data;

    if (S->bound_params) {
        array_list_deallocate(S->bound_params);
    }
    PDO_DBG_INF("number of columns: %d", stmt->column_count);
    if (S->bound_result) {
        int i;
        for (i = 0; i < stmt->column_count; ++i) {
            /* free individual value */
            efree(S->bound_result[i].value);
        }
        /* free the array of results */
        efree(S->bound_result);
    }

    snowflake_stmt_close(S->stmt);
    efree(S);
    stmt->driver_data = NULL;
    PDO_DBG_RETURN(1);
}
/* }}} */

/**
 * Execute a prepared statement. This is called by pdo_snowflake_stmt_execute.
 * 
 * Binds parameters and columns if required.
 * 
 * @param stmt Pointer to the statement structure initialized by handle_preparer.
 * @return 1 if success or 0 if error occurs
 */
static int pdo_snowflake_stmt_execute_prepared(pdo_stmt_t *stmt) /* {{{ */
{
    PDO_DBG_ENTER("pdo_snowflake_stmt_execute_prepared");
    int i;
    pdo_snowflake_stmt *S = stmt->driver_data;
    pdo_snowflake_db_handle *H = S->H;

    // TODO: bind parameters. Do we need this?
    /* S->bound_params = NULL;*/

    /* execute */
    if (snowflake_execute(S->stmt) != SF_STATUS_SUCCESS) {
        pdo_snowflake_error_stmt(stmt);
        PDO_DBG_RETURN(0);
    }

    /* Bind Columns/Results before fetching */
    stmt->column_count = (int) snowflake_num_fields(S->stmt);
    PDO_DBG_INF("number of columns: %d", stmt->column_count);
    S->bound_result = ecalloc((size_t) stmt->column_count,
                              sizeof(SF_BIND_OUTPUT));
    for (i = 0; i < stmt->column_count; ++i) {
        size_t len = 0;
        SF_COLUMN_DESC *desc = S->stmt->desc[i];
        S->bound_result[i].idx = (size_t) i + 1;  /* 1 based index */
        S->bound_result[i].type = SF_C_TYPE_STRING; /* string type */
        PDO_DBG_INF("name: %s, prec: %d, scale: %d, type: %d, c_type: %d, "
                      "byte_size: %ld, internal_bytes: %ld, null_ok: %d",
                    desc->name, desc->precision, desc->scale,
                    desc->type, desc->c_type,
                    desc->byte_size, desc->internal_size, desc->null_ok);
        if (desc->type == SF_TYPE_FIXED) {
            if (desc->scale == 0) {
                /* No decimal point but integer */
                len = (size_t) desc->precision;
            } else {
                /* The total number of digits plus decimal point */
                len = (size_t) desc->precision + 1;
            }
        } else if (desc->type == SF_TYPE_TEXT) {
            len = (size_t) desc->byte_size;
        }
        // TODO: S->bound_result[i].type =
        S->bound_result[i].max_length = len; // change this
        S->bound_result[i].value = ecalloc(len, sizeof(char));
        S->bound_result[i].len = 0; // reset the actual value length
        snowflake_bind_result(S->stmt, &S->bound_result[i]);
    }

    _pdo_snowflake_stmt_set_row_count(stmt);
    PDO_DBG_RETURN(1);
}
/* }}} */

/**
 * Execute the prepared SQL statement in the passed statement object.
 * @param stmt pdo_stmt_t
 * @return 1 if success or 0 if error occurs
 */
static int pdo_snowflake_stmt_execute(pdo_stmt_t *stmt) /* {{{ */
{
    pdo_snowflake_stmt *S = (pdo_snowflake_stmt *) stmt->driver_data;
    pdo_snowflake_db_handle *H = S->H;
    PDO_DBG_ENTER("pdo_snowflake_stmt_execute");
    PDO_DBG_INF("stmt=%p", S->stmt);

    if (S->stmt) {
        PDO_DBG_RETURN(pdo_snowflake_stmt_execute_prepared(stmt));
    }

    // TODO: stmt->active_query_stringlen should be specified.
    if (snowflake_query(S->stmt, stmt->active_query_string,
                        stmt->active_query_stringlen) !=
        SF_STATUS_SUCCESS) {
        PDO_DBG_RETURN(0);
    }
    PDO_DBG_RETURN(1);
}
/* }}} */

/**
 * Fetch a row from a previously executed statement object.
 *
 * The results of this fetch are driver dependent and the data is usually
 * stored in the driver_data member of the pdo_stmt_t object. The ori and
 * offset parameters are only meaningful if the statement represents a
 * scrollable cursor. This function returns 1 for success or 0 in the event
 * of failure.
 *
 * @param stmt Pointer to the statement structure initialized by handle_preparer.
 * @param ori One of PDO_FETCH_ORI_xxx which will determine which row will
 * be fetched.
 * @param offset If ori is set to PDO_FETCH_ORI_ABS or PDO_FETCH_ORI_REL,
 * offset represents the row desired or the row relative to the current
 * position, respectively. Otherwise, this value is ignored.
 * @return 1 if success or 0 if error occurs
 */
static int pdo_snowflake_stmt_fetch(
  pdo_stmt_t *stmt,
  enum pdo_fetch_orientation ori, zend_long offset) /* {{{ */
{
    PDO_DBG_ENTER("pdo_snowflake_stmt_fetch");
    PDO_DBG_INF("ori: %d, offset: %d", ori, offset);
    pdo_snowflake_stmt *S = (pdo_snowflake_stmt *) stmt->driver_data;
    if (ori != PDO_FETCH_ORI_NEXT) {

    }
    SF_STATUS ret = snowflake_fetch(S->stmt);
    if (ret == SF_STATUS_EOL) {
        PDO_DBG_INF("EOL");
        PDO_DBG_RETURN(0);
    } else if (ret != SF_STATUS_SUCCESS) {
        PDO_DBG_INF("ERROR 1");
        PDO_DBG_RETURN(0);
    }
    PDO_DBG_RETURN(1);
}
/* }}} */

/**
 * Query information about a particular column.
 *
 * The driver should populate the pdo_stmt_t member columns(colno) with the
 * appropriate information. This function returns 1 for success or 0 in the
 * event of failure.
 *
 * @param stmt Pointer to the statement structure initialized by handle_preparer.
 * @param colno The column number to be queried.
 * @return 1 if success or 0 if error occurs
 */
static int pdo_snowflake_stmt_describe(pdo_stmt_t *stmt, int colno) /* {{{ */
{
    int i;
    pdo_snowflake_stmt *S = (pdo_snowflake_stmt *) stmt->driver_data;
    struct pdo_column_data *cols = stmt->columns;
    PDO_DBG_ENTER("pdo_snowflake_stmt_describe");
    PDO_DBG_INF("colno %d", colno);
    if (colno >= stmt->column_count) {
        /* error invalid column */
        PDO_DBG_ERR("invalid column number. max+1: %d, colno: %d",
                    stmt->column_count, colno);
        PDO_DBG_RETURN(0);
    }
    for (i = 0; i < stmt->column_count; i++) {
        cols[i].precision = (zend_ulong) S->stmt->desc[i]->precision;
        cols[i].maxlen = (size_t) S->stmt->desc[i]->byte_size; /* TODO: str size? */
        cols[i].name = zend_string_init(
          S->stmt->desc[i]->name, strlen(S->stmt->desc[i]->name), 0);
        cols[i].param_type = PDO_PARAM_STR; /* Always string */
    }
    PDO_DBG_RETURN(1);
}
/* }}} */

/**
 * Retrieve data from the specified column.
 *
 * The driver should return the resultant data and length of that data in the
 * ptr and len variables respectively. It should be noted that the main PDO
 * driver expects the driver to manage the lifetime of the data. This function
 * returns 1 for success or 0 in the event of failure.
 *
 * @param stmt Pointer to the statement structure initialized by handle_preparer.
 * @param colno The column number to be queried.
 * @param ptr Pointer to the retrieved data.
 * @param len The length of the data pointed to by ptr.
 * @param caller_frees If set, ptr should point to emalloc'd memory and the
 *        main PDO driver will free it as soon as it is done with it.
 *        Otherwise, it will be the responsibility of the driver to free any
 *        allocated memory as a result of this call.
 * @return 1 if success or 0 if error occurs
 */
static int
pdo_snowflake_stmt_get_col(pdo_stmt_t *stmt, int colno, char **ptr, size_t *len,
                           int *caller_frees) /* {{{ */
{
    PDO_DBG_ENTER("pdo_snowflake_stmt_get_col");
    pdo_snowflake_stmt *S = (pdo_snowflake_stmt *) stmt->driver_data;
    PDO_DBG_INF("idx %d", colno);
    if (colno >= stmt->column_count) {
        /* error invalid column */
        PDO_DBG_ERR("ERROR 3");
        PDO_DBG_RETURN(0);
    }
    *ptr = S->bound_result[colno].value;
    *len = S->bound_result[colno].len;
    PDO_DBG_INF("value: '%.*s', len: %d", *len, *ptr, *len);
    PDO_DBG_RETURN(1);
}
/* }}} */

/**
 * Bind parameters or columns.
 *
 * This hook will be called for each bound parameter and bound column in the
 * statement. For ALLOC and FREE events, a single call will be made for each
 * parameter or column. The param structure contains a driver_data field that
 * the driver can use to store implementation specific information about each
 * of the parameters.
 *
 * @param stmt Pointer to the statement structure initialized by handle_preparer.
 * @param param The structure describing either a statement parameter or a bound column.
 * @param event_type The type of event to occur for this parameter, one of the following:
 *         PDO_PARAM_EVT_ALLOC
 *              Called when PDO allocates the binding. Occurs as part of
 *              PDOStatement::bindParam(), PDOStatement::bindValue() or as
 *              part of an implicit bind when calling PDOStatement::execute().
 *              This is your opportunity to take some action at this point;
 *              drivers that implement native prepared statements will
 *              typically want to query the parameter information, reconcile
 *              the type with that requested by the script, allocate an
 *              appropriately sized buffer and then bind the parameter to that
 *              buffer. You should not rely on the type or value of the zval
 *              at param->parameter at this point in time.
 *         PDO_PARAM_EVT_FREE
 *              Called once per parameter as part of cleanup. You should
 *              release any resources associated with that parameter now.
 *         PDO_PARAM_EXEC_PRE
 *              Called once for each parameter immediately before calling
 *              SKEL_stmt_execute; take this opportunity to make any final
 *              adjustments ready for execution. In particular, you should
 *              note that variables bound via PDOStatement::bindParam() are
 *              only legal to touch now, and not any sooner.
 *         PDO_PARAM_EXEC_POST
 *              Called once for each parameter immediately after calling
 *              SKEL_stmt_execute; take this opportunity to make any
 *              post-execution actions that might be required by your driver.
 *         PDO_PARAM_FETCH_PRE
 *              Called once for each parameter immediately prior to calling
 *              SKEL_stmt_fetch.
 *         PDO_PARAM_FETCH_POST
 *              Called once for each parameter immediately after calling
 *              SKEL_stmt_fetch.
 * @return 1 if success or 0 if error occurs
 */
static int pdo_snowflake_stmt_param_hook(
  pdo_stmt_t *stmt,
  struct pdo_bound_param_data *param,
  enum pdo_param_event event_type) /* {{{ */
{
    pdo_snowflake_stmt *S = (pdo_snowflake_stmt *) stmt->driver_data;
    zval *parameter = NULL;
    PDO_DBG_ENTER("pdo_snowflake_stmt_param_hook");
    PDO_DBG_INF("event = %s", pdo_param_event_names[event_type]);

    if (S->stmt == NULL) {
        /* internal error */
        PDO_DBG_RETURN(0);
    }
    if (!param->is_param) {
        /* is not parameter? */
        PDO_DBG_RETURN(0);
    }
    if (Z_ISREF(param->parameter)) {
        parameter = Z_REFVAL(param->parameter);
    } else {
        parameter = &param->parameter;
    }
    SF_BIND_INPUT *v;
    switch (event_type) {
        case PDO_PARAM_EVT_ALLOC:
            PDO_DBG_INF(
              "paramno: %ld, name: %s, max_len: %ld, type: %s, value: %p",
              param->paramno, param->name,
              param->max_value_len,
              pdo_param_type_names[param->param_type],
              parameter);

            /* sanity check parameter number range */
            if (param->paramno < 0) {
                strcpy(stmt->error_code, "HY093");
                PDO_DBG_RETURN(0);
            }
            if (S->bound_params == NULL) {
                S->bound_params = array_list_init();
            }
            v = ecalloc(1, sizeof(SF_BIND_INPUT));
            /* TODO: check if already set in the array */
            array_list_set(S->bound_params, v, (size_t) param->paramno + 1);
            break;
        case PDO_PARAM_EVT_EXEC_PRE:
            v = array_list_get(S->bound_params, (size_t) param->paramno + 1);
            v->idx = (size_t) param->paramno + 1;
            snowflake_bind_param(S->stmt, v);

            PDO_DBG_INF("%s", php_zval_type_names[Z_TYPE_P(parameter)]);
            switch (param->param_type) {
                case PDO_PARAM_INT:
                    PDO_DBG_INF(
                      "value: %ld",
                      Z_LVAL_P(parameter));
                    v->c_type = SF_C_TYPE_INT64;
                    v->len = sizeof(int64);
                    v->value = ecalloc(1, sizeof(int64));
                    *(int64 *) (v->value) = Z_LVAL_P(parameter);
                    break;
                case PDO_PARAM_STR:
                    PDO_DBG_INF(
                      "value: %.*s",
                      Z_STRLEN_P(parameter),
                      Z_STRVAL_P(parameter));
                    v->c_type = SF_C_TYPE_STRING;
                    v->len = Z_STRLEN_P(parameter);
                    v->value = Z_STRVAL_P(parameter);
                    break;
                default:
                    /* TODO: error not supported */
                    PDO_DBG_RETURN(0);
            }
            break;
        case PDO_PARAM_EVT_FREE:
            v = array_list_get(S->bound_params, (size_t) param->paramno + 1);
            switch (param->param_type) {
                case PDO_PARAM_INT:
                    efree(v->value);
                    break;
                case PDO_PARAM_STR:
                    /* nop */
                    break;
                default:
                    /* TODO: nop */
                    break;
            }
            efree(v);
            array_list_set(S->bound_params, NULL, (size_t) param->paramno + 1);
        case PDO_PARAM_EVT_EXEC_POST:
        case PDO_PARAM_EVT_FETCH_PRE:
        case PDO_PARAM_EVT_FETCH_POST:
        case PDO_PARAM_EVT_NORMALIZE:
            /* do nothing */
            break;
        default:
            break;
    }

    PDO_DBG_RETURN(1);
}
/* }}} */

/**
 * Retrieve meta data from the specified column.
 *
 * @param stmt Pointer to the statement structure initialized by handle_preparer.
 * @param colno The column number for which data is to be retrieved.
 * @param return_value Holds the returned meta data.
 * @return 1 if success or 0 if error occurs
 */
static int pdo_snowflake_stmt_col_meta(
  pdo_stmt_t *stmt, zend_long colno,
  zval *return_value) /* {{{ */
{
    PDO_DBG_ENTER("pdo_snowflake_stmt_col_meta");
    PDO_DBG_RETURN(0);
}
/* }}} */

/**
 * Advances the statement to the next rowset of the batch.
 * If it returns 1, PDO will tear down its idea of columns
 * and meta data.  If it returns 0, PDO will indicate an error
 * to the caller.
 *
 * @param stmt Pointer to the statement structure initialized by handle_preparer.
 * @return 1 if success or 0 if error occurs
 */
static int pdo_snowflake_stmt_next_rowset(pdo_stmt_t *stmt) /* {{{ */
{
    PDO_DBG_ENTER("pdo_snowflake_stmt_next_rowset");
    PDO_DBG_RETURN(1);
}
/* }}} */

/**
 * Close the active cursor on a statement, leaving the prepared
 * statement ready for re-execution.  Useful to explicitly state
 * that you are done with a given rowset, without having to explicitly
 * fetch all the rows.
 *
 * @param stmt Pointer to the statement structure initialized by handle_preparer.
 * @return 1 if success or 0 if error occurs
 */
static int pdo_snowflake_stmt_cursor_closer(pdo_stmt_t *stmt) /* {{{ */
{
    PDO_DBG_ENTER("pdo_snowflake_stmt_cursor_closer");
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
