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

#include <jmorecfg.h>
#include "libsnowflakeclient/lib/snowflake_client_version.h"

typedef char int8;
typedef unsigned char uint8;
typedef unsigned long int uint32;
typedef long int int32;
typedef unsigned long long int uint64;
typedef long long int int64;
typedef double float64;
typedef float float32;

/**
 * Snowflake Data types
 */
typedef enum sf_type {
    SF_TYPE_FIXED,
    SF_TYPE_REAL,
    SF_TYPE_TEXT,
    SF_TYPE_DATE,
    SF_TYPE_TIMESTAMP_LTZ,
    SF_TYPE_TIMESTAMP_NTZ,
    SF_TYPE_TIMESTAMP_TZ,
    SF_TYPE_VARIANT,
    SF_TYPE_OBJECT,
    SF_TYPE_ARRAY,
    SF_TYPE_BINARY,
    SF_TYPE_TIME,
    SF_TYPE_BOOLEAN
} SNOWFLAKE_TYPE;

/**
 * C data types
 */
typedef enum sf_c_type {
    SF_C_TYPE_INT8,
    SF_C_TYPE_UINT8,
    SF_C_TYPE_INT64,
    SF_C_TYPE_UINT64,
    SF_C_TYPE_STRING,
    SF_C_TYPE_TIMESTAMP
} SNOWFLAKE_C_TYPE;

/**
 * Snowflake API status
 */
typedef enum sf_status {
    SF_STATUS_SUCCESS,
    SF_STATUS_ERROR,
    SF_STATUS_WARNING,
    SF_STATUS_EOL
} SNOWFLAKE_STATUS;

/**
 * Attributes for Snowflake database session context.
 */
typedef enum sf_attribute {
    SF_CON_ACCOUNT,
    SF_CON_USER,
    SF_CON_PASSWORD,
    SF_CON_DATABASE,
    SF_CON_SCHEMA,
    SF_CON_WAREHOUSE,
    SF_CON_ROLE,
    SF_CON_HOST,
    SF_CON_PORT,
    SF_CON_PROTOCOL,
    SF_CON_PASSCODE,
    SF_CON_PASSCODE_IN_PASSWORD,
    SF_CON_APPLICATION,
    SF_CON_AUTHENTICATOR,
    SF_CON_INSECURE_MODE,
    SF_SESSION_PARAMETER,
    SF_CON_LOGIN_TIMEOUT,
    SF_CON_NETWORK_TIMEOUT,
    SF_CON_AUTOCOMMIT
} SNOWFLAKE_ATTRIBUTE;

/**
 * Attributes for Snowflake statement context.
 */
typedef enum sf_stmt_attribute {
    INTERNAL
} SNOWFLAKE_STMT_ATTRIBUTE;

/**
 * Snowflake Error
 */
typedef struct sf_error {
    int errno;
    const char *msg;
    const char *sfqid;
} SNOWFLAKE_ERROR;

/**
 * Snowflake database session context.
 */
typedef struct st_snowflake_connection {
    const char *account;
    const char *user;
    const char *password;
    const char *database;
    const char *schema;
    const char *warehouse;
    const char *role;
    const char *host;
    const char *port;
    const char *protocol;

    const char *passcode;
    boolean passcode_in_password;
    boolean insecure_mode;
    boolean autocommit;

    // Session info
    char *token;
    char *master_token;

    int64 login_timeout;
    int64 network_timeout;
} SNOWFLAKE;

/**
 * Statement context
 */
typedef struct sf_snowflake_statement {
    /* TODO */
    char *sfqid;
    SNOWFLAKE_ERROR error;
    SNOWFLAKE *connection;
} SNOWFLAKE_STMT;

/**
 * Bind input parameter context
 */
typedef struct sf_snowflake_input
{
  int idx;
  SNOWFLAKE_C_TYPE type;
  void *value;
} SNOWFLAKE_BIND_INPUT;

/**
 * Bind output parameter context
 */
typedef struct sf_snowflake_output
{
  int idx;
  SNOWFLAKE_C_TYPE type;
  void *value;
} SNOWFLAKE_BIND_OUTPUT;

/**
 * Constants
 */
extern int8 SF_BOOLEAN_TRUE;
extern int8 SF_BOOLEAN_FALSE;
extern const char EMPTY_STRING[];
extern const char CONTENT_TYPE_APPLICATION_JSON[];
extern const char ACCEPT_TYPE_APPLICATION_SNOWFLAKE[];
extern const char C_API_USER_AGENT[];


/**
 * Global Snowflake initialization.
 *
 * @return 0 if successful, errno otherwise
 */
SNOWFLAKE_STATUS STDCALL snowflake_global_init();

/**
 * Global Snowflake cleanup.
 *
 * @return 0 if successful, errno otherwise
 */
SNOWFLAKE_STATUS STDCALL snowflake_global_cleanup();

/**
 * Initializes a SNOWFLAKE connection context
 *
 * @return SNOWFLAKE context if success
 */
SNOWFLAKE *STDCALL snowflake_init();

/**
 * Purge a SNOWFLAKE connection context
 *
 * @param sf SNOWFLAKE context. The data will be freed from memory.
 */
void STDCALL snowflake_term(SNOWFLAKE *sf);

/**
 * Creates a new session and connects to Snowflake database.
 *
 * @param sf SNOWFLAKE context.
 * @return 0 if success, otherwise an errno is returned.
 */
SNOWFLAKE_STATUS STDCALL snowflake_connect(SNOWFLAKE *sf);

/**
 * Drops a session and disconnects with Snowflake database.
 *
 * @param sf SNOWFLAKE context. The data will be freed from memory
 * if the context was allocated by the library, otherwise the application is
 * responsible for freeing the data.
 */
SNOWFLAKE_STATUS STDCALL snowflake_close(SNOWFLAKE *sf);

/**
 * Sets the attribute to the session.
 *
 * @param sf SNOWFLAKE context.
 * @param type the attribute name type
 * @param value pointer to the attribute value
 * @return 0 if success, otherwise an errno is returned.
 */
SNOWFLAKE_STATUS STDCALL snowflake_set_attr(
        SNOWFLAKE *sf, SNOWFLAKE_ATTRIBUTE type, const void *value);

/**
 * Gets the attribute value from the session.
 *
 * @param sf SNOWFLAKE context.
 * @param type the attribute name type
 * @param value pointer to the attribute value buffer
 * @return 0 if success, otherwise an errno is returned.
 */
SNOWFLAKE_STATUS STDCALL snowflake_get_attr(
        SNOWFLAKE *sf, SNOWFLAKE_ATTRIBUTE type, void *value);

/**
 * Creates sf SNOWFLAKE_STMT context.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 */
SNOWFLAKE_STMT *STDCALL snowflake_stmt(SNOWFLAKE *sf);

/**
 * Closes a statement.
 * @param sfstmt SNOWFLAKE_STMT context.
 * @return 0 if success, otherwise an errno is returned.
 */
void STDCALL snowflake_stmt_close(SNOWFLAKE_STMT *sfstmt);

/**
 * Begins a new transaction.
 *
 * @param sf SNOWFLAKE context.
 * @return 0 if success, otherwise an errno is returned.
 */
SNOWFLAKE_STATUS STDCALL snowflake_trans_begin(SNOWFLAKE *sf);

/**
 * Commits a current transaction.
 *
 * @param sf SNOWFLAKE context.
 * @return 0 if success, otherwise an errno is returned.
 */
SNOWFLAKE_STATUS STDCALL snowflake_trans_commit(SNOWFLAKE *sf);

/**
 * Rollbacks a current transaction.
 *
 * @param sf SNOWFLAKE context.
 * @return 0 if success, otherwise an errno is returned.
 */
SNOWFLAKE_STATUS STDCALL snowflake_trans_rollback(SNOWFLAKE *sf);

/**
 * Returns an error message for the SNOWFLAKE context.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @return error message
 */
SNOWFLAKE_ERROR *STDCALL snowflake_error(SNOWFLAKE_STMT *sfstmt);

/**
 * Executes a query and returns result set. This function works only for
 * queries and commands that return result set. If no result set is returned,
 * NULL is returned.
 *
 * @param sf SNOWFLAKE_STMT context.
 * @param command a query or command that returns results.
 * @return 0 if success, otherwise an errno is returned.
 */
SNOWFLAKE_STATUS STDCALL snowflake_query(SNOWFLAKE_STMT *sfstmt, const char *command);

/**
 * Returns the number of affected rows in the last execution.  This function
 * works only for DML, i.e., INSERT, UPDATE, DELETE, MULTI TABLE INSERT, MERGE
 * and COPY.
 *
 * @param sf SNOWFLAKE_STMT context.
 * @return the number of affected rows
 */
int64 STDCALL snowflake_affected_rows(SNOWFLAKE_STMT *sfstmt);

/**
 * Returns the number of rows can be fetched from the result set.
 *
 * @param sfstmt SNOWFLAKE_RESULTSET context.
 * @return the number of rows.
 */
uint64 STDCALL snowflake_num_rows(SNOWFLAKE_STMT *sfstmt);

/**
 * Returns the number of fields in the result set.
 *
 * @param sfstmt SNOWFLAKE_RESULTSET context.
 * @return the number of fields.
 */
uint64 STDCALL snowflake_num_fields(SNOWFLAKE_STMT *sfstmt);

/**
 * Returns a SQLState for the result set.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @return SQL State
 */
const char *STDCALL snowflake_sqlstate(SNOWFLAKE_STMT *sfstmt);

/**
 * Prepares a statement.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @param command a query or command that returns results.
 * @return 0 if success, otherwise an errno is returned.
 */
SNOWFLAKE_STATUS STDCALL snowflake_prepare(
        SNOWFLAKE_STMT *sfstmt, const char *command);

/**
 * Sets the statement attribute to the session.
 *
 * @param sf SNOWFLAKE context.
 * @param type the attribute name type
 * @param value pointer to the attribute value
 * @return 0 if success, otherwise an errno is returned.
 */
SNOWFLAKE_STATUS STDCALL snowflake_stmt_set_attr(
        SNOWFLAKE_STMT *sf, SNOWFLAKE_STMT_ATTRIBUTE type, const void *value);

/**
 * Gets the statement attribute value from the session.
 *
 * @param sf SNOWFLAKE context.
 * @param type the attribute name type
 * @param value pointer to the attribute value buffer
 * @return 0 if success, otherwise an errno is returned.
 */
SNOWFLAKE_STATUS STDCALL snowflake_stmt_get_attr(
        SNOWFLAKE_STMT *sf, SNOWFLAKE_STMT_ATTRIBUTE type, void *value);

/**
 * Executes a statement.
 * @param sfstmt SNOWFLAKE_STMT context.
 *
 * @return 0 if success, otherwise an errno is returned.
 */
SNOWFLAKE_STATUS STDCALL snowflake_execute(SNOWFLAKE_STMT *sfstmt);

/**
 * Fetches the next row for the statement and stores on the bound buffer
 * if any. Noop if no buffer is bound.
 *
 * @param sfstmt SNOWFLAKE_RESULTSET context.
 * @return 0 if success, otherwise an errno is returned.
 */
SNOWFLAKE_STATUS STDCALL snowflake_fetch(SNOWFLAKE_STMT *sfres);

/**
 * Returns the number of binding parameters in the statement.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @return the number of binding parameters in the statement.
 */
uint64 STDCALL snowflake_param_count(SNOWFLAKE_STMT *sfstmt);

/**
 * Binds parameters with the statement for execution.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @param sfbind SNOWFLAKE_BIND_INPUT context array.
 * @return 0 if success, otherwise an errno is returned.
 */
SNOWFLAKE_STATUS STDCALL snowflake_bind_param(
    SNOWFLAKE_STMT *sfstmt, SNOWFLAKE_BIND_INPUT *sfbind);

/**
 * Binds buffers with the statement for result set processing.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @param sfbind_array SNOWFLAKE_BIND_OUTPUT context array.
 * @return 0 if success, otherwise an errno is returned.
 */
SNOWFLAKE_STATUS STDCALL snowflake_bind_result(
    SNOWFLAKE_STMT *sfstmt, SNOWFLAKE_BIND_OUTPUT *sfbind);

/**
 * Returns a query id associated with the statement after execution. If not
 * executed, NULL is returned.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @return query id associated with the statement.
 */
const char *STDCALL snowflake_sfqid(SNOWFLAKE_STMT *sfstmt);


#ifdef  __cplusplus
}
#endif

#endif //PDO_SNOWFLAKE_SNOWFLAKE_CLIENT_H
