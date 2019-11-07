/* Copyright (c) 2017-2019 Snowflake Computing Inc. All right reserved.  */

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

    PDO_LOG_ENTER("_pdo_snowflake_stmt_set_row_count");

    row_count = (zend_long) snowflake_affected_rows(S->stmt);
    PDO_LOG_DBG("row count: %lld", row_count);
    if (row_count != (zend_long) -1) {
        stmt->row_count = row_count;
    }
}
/* }}} */

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
    PDO_LOG_ENTER("pdo_snowflake_stmt_dtor");
    pdo_snowflake_stmt *S = stmt->driver_data;

    if (S->bound_params) {
        pdo_sf_param_store_deallocate(S->bound_params);
    }

    // Release string bindings
    if (S->bound_results) {
        for(int i = 0; i < stmt->column_count; i++) {
            efree(S->bound_results[i].value);
        }
        efree(S->bound_results);
    }

    PDO_LOG_DBG("number of columns: %d", stmt->column_count);
    snowflake_stmt_term(S->stmt);
    efree(S);
    stmt->driver_data = NULL;
    PDO_LOG_RETURN(1);
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
    PDO_LOG_ENTER("pdo_snowflake_stmt_execute_prepared");
    int i;
    pdo_snowflake_stmt *S = stmt->driver_data;

    /* execute */
    if (snowflake_execute(S->stmt) != SF_STATUS_SUCCESS) {
        pdo_snowflake_error_stmt(stmt);
        PDO_LOG_RETURN(0);
    }

    /* Bind Columns/Results before fetching */
    stmt->column_count = (int) snowflake_num_fields(S->stmt);
    PDO_LOG_DBG("number of columns: %d", stmt->column_count);
    // Create an array of string structs
    S->bound_results = ecalloc((size_t) stmt->column_count, sizeof(pdo_snowflake_string));

    for(i = 0; i < stmt->column_count; i++) {
        S->bound_results[i].value = NULL;
        S->bound_results[i].size = 0;
    }

    _pdo_snowflake_stmt_set_row_count(stmt);
    PDO_LOG_RETURN(1);
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
    PDO_LOG_ENTER("pdo_snowflake_stmt_execute");

    if (S->stmt) {
        int ret = pdo_snowflake_stmt_execute_prepared(stmt);
        PDO_LOG_RETURN(ret);
    }

    // TODO: stmt->active_query_stringlen should be specified.
    if (snowflake_query(S->stmt, stmt->active_query_string,
                        stmt->active_query_stringlen) !=
        SF_STATUS_SUCCESS) {
        PDO_LOG_RETURN(0);
    }
    PDO_LOG_RETURN(1);
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
    PDO_LOG_ENTER("pdo_snowflake_stmt_fetch");
    PDO_LOG_DBG("ori: %d, offset: %d", ori, offset);
    pdo_snowflake_stmt *S = (pdo_snowflake_stmt *) stmt->driver_data;
    if (ori != PDO_FETCH_ORI_NEXT) {
        /* TODO: raise error */
    }
    SF_STATUS ret = snowflake_fetch(S->stmt);
    if (ret == SF_STATUS_EOF) {
        PDO_LOG_DBG("EOL");
        PDO_LOG_RETURN(0);
    } else if (ret != SF_STATUS_SUCCESS) {
        PDO_LOG_DBG("ERROR 1");
        PDO_LOG_RETURN(0);
    }
    PDO_LOG_RETURN(1);
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
    PDO_LOG_ENTER("pdo_snowflake_stmt_describe");
    PDO_LOG_DBG("colno %d", colno);
    if (colno >= stmt->column_count) {
        /* error invalid column */
        PDO_LOG_ERR("invalid column number. max+1: %d, colno: %d",
                    stmt->column_count, colno);
        PDO_LOG_RETURN(0);
    }
    SF_COLUMN_DESC *F = snowflake_desc(S->stmt);
    for (i = 0; i < stmt->column_count; i++) {
        cols[i].precision = (zend_ulong) F[i].precision;
        switch (F[i].type) {
            case SF_DB_TYPE_OBJECT:
            case SF_DB_TYPE_ARRAY:
            case SF_DB_TYPE_VARIANT:
                /* No size is given from the server */
                cols[i].maxlen = SF_MAX_OBJECT_SIZE;
                break;
            case SF_DB_TYPE_BOOLEAN:
                cols[i].maxlen =
                  (sizeof(SF_BOOLEAN_TRUE_STR) > sizeof(SF_BOOLEAN_FALSE_STR)
                   ? sizeof(SF_BOOLEAN_TRUE_STR)
                   : sizeof(SF_BOOLEAN_FALSE_STR)) - 1;
                break;
            case SF_DB_TYPE_BINARY:
                cols[i].maxlen = (size_t) F[i].byte_size;
                break;
            case SF_DB_TYPE_DATE:
            case SF_DB_TYPE_TIMESTAMP_NTZ:
            case SF_DB_TYPE_TIMESTAMP_TZ:
            case SF_DB_TYPE_TIMESTAMP_LTZ:
            case SF_DB_TYPE_TIME:
                /* length doesn't matter to allocate buffer */
                cols[i].maxlen = (size_t) F[i].byte_size;
                break;
            default:
                cols[i].maxlen = (size_t) F[i].byte_size;
                break;
        }
        cols[i].name = zend_string_init(
          F[i].name, strlen(F[i].name), 0);
        cols[i].param_type = PDO_PARAM_STR; /* Always string */
    }
    PDO_LOG_RETURN(1);
}
/* }}} */

/**
 * Retrieve data from the specified column.
 *
 * The driver should return the result data and length of that data in the
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
    PDO_LOG_ENTER("pdo_snowflake_stmt_get_col");
    pdo_snowflake_stmt *S = (pdo_snowflake_stmt *) stmt->driver_data;
    if (colno >= stmt->column_count) {
        /* error invalid column */
        /* TODO */
        PDO_LOG_ERR("ERROR 3");
        PDO_LOG_RETURN(0);
    }
    sf_bool is_null;
    snowflake_column_is_null(S->stmt, colno + 1, &is_null);
    if (is_null) {
        *ptr = NULL;
        *len = 0;
    } else {
        size_t value_len = 0;
        pdo_snowflake_string *str = &(S->bound_results[colno]);
        snowflake_column_as_str(S->stmt, colno + 1, &str->value, &value_len, &str->size);
        *ptr = str->value;
        *len = value_len;
    }
    PDO_LOG_DBG("idx: %d, value: '%.*s', len: %d", colno, *len, *ptr, *len);
    PDO_LOG_RETURN(1);
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
 *              stmt_execute; take this opportunity to make any final
 *              adjustments ready for execution. In particular, you should
 *              note that variables bound via PDOStatement::bindParam() are
 *              only legal to touch now, and not any sooner.
 *         PDO_PARAM_EXEC_POST
 *              Called once for each parameter immediately after calling
 *              stmt_execute; take this opportunity to make any
 *              post-execution actions that might be required by your driver.
 *         PDO_PARAM_FETCH_PRE
 *              Called once for each parameter immediately prior to calling
 *              stmt_fetch.
 *         PDO_PARAM_FETCH_POST
 *              Called once for each parameter immediately after calling
 *              stmt_fetch.
 * @return 1 if success or 0 if error occurs
 */
static int pdo_snowflake_stmt_param_hook(
  pdo_stmt_t *stmt,
  struct pdo_bound_param_data *param,
  enum pdo_param_event event_type) /* {{{ */
{
    int ret = 1;
    pdo_snowflake_stmt *S = (pdo_snowflake_stmt *) stmt->driver_data;
    zval *parameter = NULL;
    PARAM_STORE *pstore = NULL;
    PDO_LOG_ENTER("pdo_snowflake_stmt_param_hook");
    PDO_LOG_DBG("event = %s, paramno: %ld",
                pdo_param_event_names[event_type], param->paramno);

    if (S->stmt == NULL) {
        /* internal error */
        PDO_LOG_RETURN(0);
    }
    if (!param->is_param) {
        /* nop if not binding parameter but column */
        PDO_LOG_RETURN(1);
    }

    if (S->bound_params != NULL)
    {
      pstore = (PARAM_STORE *) S->bound_params;
      if ((param->paramno > -1 && pstore->param_style == NAMED) ||
          (param->paramno < 0 && pstore->param_style == POSITIONAL))
      {
        pdo_raise_impl_error(stmt->dbh, stmt, SF_SQLSTATE_INVALID_PARAMETER_TYPE,
                             "Mixing Named and Positional parameter is not allowed in Snowflake PDO Driver");
        PDO_LOG_RETURN(0);
      }
    }

    if (Z_ISREF(param->parameter)) {
        parameter = Z_REFVAL(param->parameter);
    } else {
        parameter = &param->parameter;
    }
    SF_BIND_INPUT *v;

    switch (event_type) {
        case PDO_PARAM_EVT_ALLOC:
            PDO_LOG_DBG(
              "paramno: %ld, name: %s, max_len: %ld, type: %s, value: %p",
              param->paramno, ZSTR_VAL(param->name),
              param->max_value_len,
              pdo_param_type_names[param->param_type],
              parameter);

            if (S->bound_params == NULL) {
                pdo_sf_param_store_init(_pdo_sf_get_param_style(param->paramno), &S->bound_params);
            }

            v = ecalloc(1, sizeof(SF_BIND_INPUT));
            /* TODO: check if already set in the paramstore*/

            pdo_sf_param_store_set(S->bound_params, v,
                    (size_t) param->paramno+1,
                    ZSTR_VAL(param->name));
            break;
        case PDO_PARAM_EVT_EXEC_PRE:
            v = pdo_sf_param_store_get(S->bound_params,
                    (size_t) param->paramno + 1,
                    ZSTR_VAL(param->name));
            if (v == NULL)
            {
              PDO_LOG_ERR("Could not retrieve param store.");
              break;
            }

            /*
             * Update the idx and name field for libsnowflakeclient
             * to infer whether parameter style is positional or named.
             *
             * Note that if Named, paramno would be -1 making idx = 0
             * which is the expected value for index by libsnowflakeclient
             */
            v->idx = (size_t) param->paramno + 1;

            if (param->name != NULL && ZSTR_VAL(param->name) != NULL)
            {
              char *name = ZSTR_VAL(param->name);
              int len = ZSTR_LEN(param->name);
              if (name[0] == ':')
              {
                name = name+1;
              }
              else
              {
                len++; // add the null terminator
              }
              v->name = ecalloc(len, sizeof(char));
              strncpy(v->name, name, len);
            }
            else
            {
              v->name = NULL;
            }

            snowflake_bind_param(S->stmt, v);

            PDO_LOG_DBG("%s", php_zval_type_names[Z_TYPE_P(parameter)]);
            if (Z_TYPE_P(parameter) == IS_NULL) {
                v->c_type = SF_C_TYPE_STRING;
                v->len = (size_t) 0;
                v->value = NULL;
                break;
            }
            switch (param->param_type) {
                case PDO_PARAM_NULL:
                    v->c_type = SF_C_TYPE_STRING;
                    v->len = (size_t) 0;
                    v->value = NULL;
                    break;
                case PDO_PARAM_INT:
                    PDO_LOG_DBG(
                      "value: %ld",
                      Z_LVAL_P(parameter));
                    v->c_type = SF_C_TYPE_INT64;
                    v->len = sizeof(int64);
                    v->value = ecalloc(1, sizeof(int64));
                    *(int64 *) (v->value) = Z_LVAL_P(parameter);
                    break;
                case PDO_PARAM_STR:
                    PDO_LOG_DBG(
                      "value: %.*s, len: %lld",
                      Z_STRLEN_P(parameter),
                      Z_STRVAL_P(parameter),
                      Z_STRLEN_P(parameter));
                    v->c_type = SF_C_TYPE_STRING;
                    v->len = Z_STRLEN_P(parameter);
                    v->value = Z_STRVAL_P(parameter);
                    break;
                case PDO_PARAM_LOB:
                    PDO_LOG_DBG(
                      "value: %.*s, len: %lld",
                      Z_STRLEN_P(parameter),
                      Z_STRVAL_P(parameter),
                      Z_STRLEN_P(parameter));
                    v->c_type = SF_C_TYPE_BINARY;
                    v->len = Z_STRLEN_P(parameter);
                    v->value = Z_STRVAL_P(parameter);
                    break;
                case PDO_PARAM_BOOL:
                    PDO_LOG_DBG(
                      "value: %s",
                      php_zval_type_names[Z_TYPE_P(parameter)]);
                    if (Z_TYPE_P(parameter) == IS_FALSE) {
                        v->value = (void *) &SF_BOOLEAN_FALSE;
                    } else {
                        v->value = (void *) &SF_BOOLEAN_TRUE;
                    }
                    v->len = sizeof(sf_bool);
                    v->c_type = SF_C_TYPE_BOOLEAN;
                    break;
                default:
                    /* TODO: error not supported */
                    ret = 0;
                    goto clean;
            }
            break;
        case PDO_PARAM_EVT_FREE:
            v = pdo_sf_param_store_get(S->bound_params,
                                       (size_t) param->paramno + 1, ZSTR_VAL(param->name));
            if (Z_TYPE_P(parameter) != IS_NULL) {
                switch (param->param_type) {
                    case PDO_PARAM_INT:
                        efree(v->value);
                        break;
                    case PDO_PARAM_STR:
                    case PDO_PARAM_BOOL:
                    case PDO_PARAM_NULL:
                    case PDO_PARAM_LOB:
                        /* nop */
                        break;
                    default:
                        /* TODO: nop */
                        break;
                }
            }
            efree(v->name);
            efree(v);
            pdo_sf_param_store_set(S->bound_params, NULL,
                    (size_t) param->paramno + 1, ZSTR_VAL(param->name));
        case PDO_PARAM_EVT_EXEC_POST:
        case PDO_PARAM_EVT_FETCH_PRE:
        case PDO_PARAM_EVT_FETCH_POST:
        case PDO_PARAM_EVT_NORMALIZE:
            /* do nothing */
            break;
        default:
            break;
    }
clean:

    PDO_LOG_RETURN(ret);
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
    PDO_LOG_ENTER("pdo_snowflake_stmt_col_meta");
    PDO_LOG_DBG("colno: %lld", colno);
    if (!stmt) {
        PDO_LOG_RETURN(0);
    }
    pdo_snowflake_stmt *S = (pdo_snowflake_stmt *) stmt->driver_data;
    SF_COLUMN_DESC *F;
    zval flags;

    if (colno >= stmt->column_count) {
        /* error invalid column */
        PDO_LOG_ERR("invalid column index: %d", colno);
        PDO_LOG_RETURN(0);
    }

    array_init(return_value);
    array_init(&flags);

    F = snowflake_desc(S->stmt);
    if (!F) {
        PDO_LOG_ERR("failed to get SF_COLUMN_DESC");
        PDO_LOG_RETURN(1);
    }
    if (!F[colno].null_ok) {
        add_next_index_string(&flags, "not_null");
    }
    add_assoc_long(return_value, "scale", (zend_long) F[colno].scale);
    add_assoc_string(return_value, "native_type",
                     (char *) snowflake_type_to_string(F[colno].type));
    add_assoc_zval(return_value, "flags", &flags);

    PDO_LOG_RETURN(1);
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
    PDO_LOG_ENTER("pdo_snowflake_stmt_next_rowset");
    /* NOP. no multiple statement is supported at the momemnt. */
    PDO_LOG_RETURN(1);
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
    PDO_LOG_ENTER("pdo_snowflake_stmt_cursor_closer");
    /* NOP. unlike other database, Snowflake doesn't need to fetch
     * all data to close the statement.
     * */
    PDO_LOG_RETURN(1);
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
