/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef PDO_SNOWFLAKE_SNOWFLAKE_CLIENT_H
#define PDO_SNOWFLAKE_SNOWFLAKE_CLIENT_H

#ifdef  __cplusplus
extern "C" {
#endif

#if !defined(_WIN32)
#define STDCALL
#else
#define STDCALL __stdcall
#endif

#include "snowflake_client_version.h"

/**
 * Maximum length of attribute value
 */
#define SNOWFLAKE_MAX_ATTRIBUTE_VALUE_LENGTH 4096

typedef double float64;
typedef float float32;
typedef unsigned long int uint32;
typedef long int int32;
typedef unsigned long long int uint64;
typedef long long int int64;

/**
 * Snowflake database session context.
 */
typedef struct st_snowflake_connection
{
  /* TODO */
} SNOWFLAKE;

/**
 * Attributes for Snowflake database session context.
 */
enum SNOWFLAKE_ATTRIBUTE
{
  LOGIN_TIMEOUT,
  REQUEST_TIMEOUT
};

/**
 * Result set context.
 */
typedef struct sf_snowflake_resultset
{
  /* TODO */
} SNOWFLAKE_RESULTSET;

/**
 * Attributes for statement context.
 */
enum SNOWFLAKE_STMT_ATTRIBUTE
{
  INTERNAL
};

/**
 * Statement context
 */
typedef struct sf_snowflake_statement
{
  /* TODO */
} SNOWFLAKE_STMT;

/**
 * Bind context
 */
typedef struct sf_snowflake_bind
{
  /* TODO */
} SNOWFLAKE_BIND;

/**
 * Creates a new session and connects to Snowflake database.
 *
 * @param sf SNOWFLAKE context. The application may pass a pointer of an
 * allocated data or NULL if the application wants the library to allocate it.
 * @param account account name
 * @param user user name
 * @param password password
 * @param options other optional parameters
 * @return SNOWFLAKE context if the connection is succes otherwise NULL.
 * The error information can be retrieved by snowflake_error.
 */
SNOWFLAKE *STDCALL snowflake_connect(
    SNOWFLAKE *sf,
    const char *account, const char *user, const char *password,
    char **options);

/**
 * Drops a session and disconnects with Snowflake database.
 *
 * @param sf SNOWFLAKE context. The data will be freed from memory
 * if the context was allocated by the library, otherwise the application is
 * responsible for freeing the data.
 */
void STDCALL snowflake_close(SNOWFLAKE *sf);

/**
 * Sets the attribute to the session.
 *
 * @param sf SNOWFLAKE context.
 * @param type the attribute name type
 * @param value pointer to the attribute value
 * @return 0 if success, otherwise an errno is returned.
 */
int STDCALL snowflake_set_attr(
    SNOWFLAKE *sf, enum SNOWFLAKE_ATTRIBUTE type, const void *value);

/**
 * Gets the attribute value from the session.
 *
 * @param sf SNOWFLAKE context.
 * @param type the attribute name type
 * @param value pointer to the attribute value buffer
 * @return 0 if success, otherwise an errno is returned.
 */
int STDCALL snowflake_get_attr(
    SNOWFLAKE *sf, enum SNOWFLAKE_ATTRIBUTE type, void *value);

/**
 * Returns an error message for the SNOWFLAKE context.
 *
 * @param sf SNOWFLAKE context.
 * @return error message
 */
const char *STDCALL snowflake_error(SNOWFLAKE *sf);

/**
 * Returns a SQLState for the SNOWFLAKE context.
 *
 * @param sf SNOWFLAKE context.
 * @return SQL State
 */
const char *STDCALL snowflake_sqlstate(SNOWFLAKE *sf);

/**
 * Begins a new transaction.
 *
 * @param sf SNOWFLAKE context.
 * @return 0 if success, otherwise an errno is returned.
 */
int *STDCALL snowflake_begin(SNOWFLAKE *sf);

/**
 * Commits a current transaction.
 *
 * @param sf SNOWFLAKE context.
 * @return 0 if success, otherwise an errno is returned.
 */
int *STDCALL snowflake_commit(SNOWFLAKE *sf);

/**
 * Rollbacks a current transaction.
 *
 * @param sf SNOWFLAKE context.
 * @return 0 if success, otherwise an errno is returned.
 */
int *STDCALL snowflake_rollback(SNOWFLAKE *sf);

/**
 * Executes a query and returns result set. This function works only for
 * queries and commands that return result set. If no result set is returned,
 * NULL is returned.
 *
 * @param sf SNOWFLAKE context.
 * @param command a query or command that returns results.
 * @return SNOWFLAKE_RESULTSET if success, otherwise NULL.
 */
SNOWFLAKE_RESULTSET *STDCALL snowflake_query(
    SNOWFLAKE *sf, const char *command);

/**
 * Returns the number of affected rows in the last execution.
 *
 * @param sf SNOWFLAKE context.
 * @return the number of affected rows
 */
int *STDCALL snowflake_affected_rows(SNOWFLAKE *sf);

/**
 * Returns the number of rows can be fetched from the result set.
 *
 * @param sfres SNOWFLAKE_RESULTSET context.
 * @return the number of rows.
 */
uint64 STDCALL snowflake_num_rows(SNOWFLAKE_RESULTSET *sfres);

/**
 * Returns the number of fields in the result set.
 *
 * @param sfres SNOWFLAKE_RESULTSET context.
 * @return the number of fields.
 */
uint64 STDCALL snowflake_num_fields(SNOWFLAKE_RESULTSET *sfres);

/**
 * Prepares a statement.
 *
 * @param sf SNOWFLAKE context.
 * @return SNOWFLAKE_STMT if success, otherwise NULL.
 */
SNOWFLAKE_STMT *STDCALL snowflake_stmt_prepare(SNOWFLAKE *sf);

/**
 * Sets the statement attribute to the session.
 *
 * @param sf SNOWFLAKE context.
 * @param type the attribute name type
 * @param value pointer to the attribute value
 * @return 0 if success, otherwise an errno is returned.
 */
int STDCALL snowflake_stmt_set_attr(
    SNOWFLAKE_STMT *sf, enum SNOWFLAKE_STMT_ATTRIBUTE type, const void *value);

/**
 * Gets the statement attribute value from the session.
 *
 * @param sf SNOWFLAKE context.
 * @param type the attribute name type
 * @param value pointer to the attribute value buffer
 * @return 0 if success, otherwise an errno is returned.
 */
int STDCALL snowflake_stmt_get_attr(
    SNOWFLAKE_STMT *sf, enum SNOWFLAKE_STMT_ATTRIBUTE type, void *value);

/**
 * Executes a statement.
 * @param sfstmt SNOWFLAKE_STMT context.
 *
 * @return 0 if success, otherwise an errno is returned.
 */
int STDCALL snowflake_stmt_execute(SNOWFLAKE_STMT *sfstmt);

/**
 * Fetches the next row for the statement and stores on the bound buffer
 * if any. Noop if no buffer is bound.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @return 0 if success, otherwise an errno is returned.
 */
int STDCALL snowflake_stmt_fetch(SNOWFLAKE_STMT *sfstmt);

/**
 * Fetches a column in the next row and stores on the bound buffer.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @param sfbind SNOWFLAKE_BIND context including bound buffer information.
 * @param column one based index.
 * @return 0 if success, otherwise an errno is returned.
 */
int STDCALL snowflake_stmt_fetch_column(
    SNOWFLAKE_STMT *sfstmt, SNOWFLAKE_BIND *sfbind, uint64 column);

/**
 * Returns the number of binding parameters in the statement.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @return the number of binding parameters in the statement.
 */
uint64 STDCALL snowflake_stmt_param_count(SNOWFLAKE_STMT *sfstmt);

/**
 * Binds parameters with the statement for execution.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @param sfbind SNOWFLAKE_BIND context array.
 * @return 0 if success, otherwise an errno is returned.
 */
int STDCALL snowflake_stmt_bind_params(
    SNOWFLAKE_STMT *sfstmt, SNOWFLAKE_BIND *sfbind_array);

/**
 * Binds buffers with the statement for result set processing.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @param sfbind_array SNOWFLAKE_BIND context array.
 * @return 0 if success, otherwise an errno is returned.
 */
int STDCALL snowflake_stmt_bind_results(
    SNOWFLAKE_STMT *sfstmt, SNOWFLAKE_BIND *sfbind_array);

/**
 * Returns a query id associated with the statement after execution. If not
 * executed, NULL is returned.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @return query id associated with the statement.
 */
const char *STDCALL snowflake_sfqid(SNOWFLAKE_STMT *sfstmt);

/**
 * Closes a  statement.
 * @param sfres SNOWFLAKE_STMT context.
 * @return 0 if success, otherwise an errno is returned.
 */
int *STDCALL snowflake_stmt_close(SNOWFLAKE_STMT *sfres);

/**
 * Returns the number of affected rows by the statement. This function works
 * only for DML, i.e., INSERT, UPDATE, DELETE, MULTI TABLE INSERT, MERGE
 * and COPY.
 *
 * @param sfres SNOWFLAKE_STMT context.
 * @return the number of affected rows.
 */
uint64 STDCALL snowflake_stmt_affected_rows(SNOWFLAKE_STMT *sfres);


#ifdef  __cplusplus
}
#endif

#endif //PDO_SNOWFLAKE_SNOWFLAKE_CLIENT_H
