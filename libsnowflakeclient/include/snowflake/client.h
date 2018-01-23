/*
 * Copyright (c) 2017-2018 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef SNOWFLAKE_CLIENT_H
#define SNOWFLAKE_CLIENT_H

#ifdef  __cplusplus
extern "C" {
#endif

#if !defined(_WIN32)
#define STDCALL
#else
#define STDCALL __stdcall
#endif

#include <pthread.h>
#include "snowflake/basic_types.h"
#include "version.h"

/**
 * API Name
 */
#define SF_API_NAME "C API"

/**
 * API Version
 */
#define SF_API_VERSION "0.1"

/**
 * SQLState code length
 */
#define SF_SQLSTATE_LEN 6

/**
 * Authenticator, Default
 */
#define SF_AUTHENTICATOR_DEFAULT "snowflake"

/**
 * Authenticator, external browser
 * TODO
 */
#define SF_AUTHENTICATOR_EXTERNAL_BROWSER "externalbrowser"

/**
 * UUID4 length
 */
#define SF_UUID4_LEN 37

/**
 * The maximum object size
 */
#define SF_MAX_OBJECT_SIZE 16777216

/**
 * Snowflake Data types
 *
 * Use snowflake_type_to_string to get the string representation.
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
} SF_TYPE;

/**
 * C data types
 *
 * Use snowflake_c_type_to_string to get the string representation.
 */
typedef enum sf_c_type {
    SF_C_TYPE_INT8,
    SF_C_TYPE_UINT8,
    SF_C_TYPE_INT64,
    SF_C_TYPE_UINT64,
    SF_C_TYPE_FLOAT64,
    SF_C_TYPE_STRING,
    SF_C_TYPE_TIMESTAMP,
    SF_C_TYPE_BOOLEAN,
    SF_C_TYPE_BINARY
} SF_C_TYPE;

/**
 * Snowflake API status
 */
typedef enum sf_status {
    // Special status are negative
      SF_STATUS_EOF = -1,
    // Success is zero
      SF_STATUS_SUCCESS = 0,
    // Errors are positive
    SF_STATUS_ERROR_GENERAL = 240000,
    SF_STATUS_ERROR_OUT_OF_MEMORY = 240001,
    SF_STATUS_ERROR_REQUEST_TIMEOUT = 240002,
    SF_STATUS_ERROR_DATA_CONVERSION = 240003,
    SF_STATUS_ERROR_BAD_DATA_OUTPUT_TYPE = 240004,
    SF_STATUS_ERROR_BAD_CONNECTION_PARAMS = 240005,
    SF_STATUS_ERROR_STRING_FORMATTING = 240006,
    SF_STATUS_ERROR_STRING_COPY = 240007,
    SF_STATUS_ERROR_BAD_REQUEST = 240008,
    SF_STATUS_ERROR_BAD_RESPONSE = 240009,
    SF_STATUS_ERROR_BAD_JSON = 240010,
    SF_STATUS_ERROR_RETRY = 240011,
    SF_STATUS_ERROR_CURL = 240012,
    SF_STATUS_ERROR_BAD_ATTRIBUTE_TYPE = 240013,
    SF_STATUS_ERROR_APPLICATION_ERROR = 240014,
    SF_STATUS_ERROR_PTHREAD = 240015,
    SF_STATUS_ERROR_CONNECTION_NOT_EXIST = 240016,
    SF_STATUS_ERROR_STATEMENT_NOT_EXIST = 240017,

} SF_STATUS;

/**
 * SQLState for client errors
 */
#define SF_SQLSTATE_NO_ERROR "00000"
#define SF_SQLSTATE_UNABLE_TO_CONNECT "08001"
#define SF_SQLSTATE_CONNECTION_ALREADY_EXIST "08002"
#define SF_SQLSTATE_CONNECTION_NOT_EXIST "08003"
#define SF_SQLSTATE_APP_REJECT_CONNECTION "08004"

// For general purpose
#define SF_SQLSTATE_NO_DATA "02000"

// For CLI specific
#define SF_SQLSTATE_GENERAL_ERROR "HY000"
#define SF_SQLSTATE_MEMORY_ALLOCATION_ERROR "HY001"
#define SF_SQLSTATE_INVALID_DATA_TYPE_IN_APPLICATION_DESCRIPTOR "HY003"
#define SF_SQLSTATE_INVALID_DATA_TYPE "HY004"
#define SF_SQLSTATE_ASSOCIATED_STATEMENT_IS_NOT_PREPARED "HY007"
#define SF_SQLSTATE_OPERATION_CANCELED "HY008"
#define SF_SQLSTATE_INVALID_USE_OF_NULL_POINTER "HY009"
#define SF_SQLSTATE_FUNCTION_SEQUENCE_ERROR "HY010"
#define SF_SQLSTATE_ATTRIBUTE_CANNOT_BE_SET_NOW "HY011"
#define SF_SQLSTATE_INVALID_TRANSACTION_OPERATION_CODE "HY012"
#define SF_SQLSTATE_MEMORY_MANAGEMENT_ERROR "HY013"
#define SF_SQLSTATE_LIMIT_ON_THE_NUMBER_OF_HANDLES_EXCEEDED "HY014"
#define SF_SQLSTATE_INVALID_USE_OF_AN_AUTOMATICALLY_ALLOCATED_DESCRIPTOR_HANDLE "HY017"
#define SF_SQLSTATE_SERVER_DECLINED_THE_CANCELLATION_REQUEST "HY018"
#define SF_SQLSTATE_NON_STRING_DATA_CANNOT_BE_SENT_IN_PIECES "HY019"
#define SF_SQLSTATE_ATTEMPT_TO_CONCATENATE_A_NULL_VALUE "HY020"
#define SF_SQLSTATE_INCONSISTENT_DESCRIPTOR_INFORMATION "HY021"
#define SF_SQLSTATE_INVALID_ATTRIBUTE_VALUE "HY024"
#define SF_SQLSTATE_NON_STRING_DATA_CANNOT_BE_USED_WITH_STRING_ROUTINE "HY055"
#define SF_SQLSTATE_INVALID_STRING_LENGTH_OR_BUFFER_LENGTH "HY090"
#define SF_SQLSTATE_INVALID_DESCRIPTOR_FIELD_IDENTIFIER "HY091"
#define SF_SQLSTATE_INVALID_ATTRIBUTE_IDENTIFIER "HY092"
#define SF_SQLSTATE_INVALID_FUNCTIONID_SPECIFIED "HY095"
#define SF_SQLSTATE_INVALID_INFORMATION_TYPE "HY096"
#define SF_SQLSTATE_COLUMN_TYPE_OUT_OF_RANGE "HY097"
#define SF_SQLSTATE_SCOPE_OUT_OF_RANGE "HY098"
#define SF_SQLSTATE_NULLABLE_TYPE_OUT_OF_RANGE "HY099"
#define SF_SQLSTATE_INVALID_RETRIEVAL_CODE "HY103"
#define SF_SQLSTATE_INVALID_LENGTHPRECISION_VALUE "HY104"
#define SF_SQLSTATE_INVALID_PARAMETER_TYPE "HY105"
#define SF_SQLSTATE_INVALID_FETCH_ORIENTATION "HY106"
#define SF_SQLSTATE_ROW_VALUE_OUT_OF_RANGE "HY107"
#define SF_SQLSTATE_INVALID_CURSOR_POSITION "HY108"
#define SF_SQLSTATE_OPTIONAL_FEATURE_NOT_IMPLEMENTED "HYC00"

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
    SF_CON_APPLICATION_NAME,
    SF_CON_APPLICATION_VERSION,
    SF_CON_AUTHENTICATOR,
    SF_CON_INSECURE_MODE,
    SF_CON_LOGIN_TIMEOUT,
    SF_CON_NETWORK_TIMEOUT,
    SF_CON_TIMEZONE,
    SF_CON_AUTOCOMMIT
} SF_ATTRIBUTE;

/**
 * Attributes for Snowflake global context.
 */
typedef enum sf_global_attribute {
    SF_GLOBAL_DISABLE_VERIFY_PEER,
    SF_GLOBAL_CA_BUNDLE_FILE,
    SF_GLOBAL_SSL_VERSION,
    SF_GLOBAL_DEBUG
} SF_GLOBAL_ATTRIBUTE;

/**
 * Attributes for Snowflake statement context.
 */
typedef enum sf_stmt_attribute {
    INTERNAL
} SF_STMT_ATTRIBUTE;

/**
 * Snowflake Error
 */
typedef struct sf_error {
    SF_STATUS error_code;
    char sqlstate[SF_SQLSTATE_LEN];
    char *msg;
    sf_bool is_shared_msg;
    char sfqid[SF_UUID4_LEN];
    char *file;
    int line;
} SF_ERROR;

/**
 * Snowflake database session context.
 */
typedef struct sf_snowflake_connection {
    char *account;
    char *user;
    char *password;
    char *database;
    char *schema;
    char *warehouse;
    char *role;
    char *host;
    char *port;
    char *protocol;

    char *passcode;
    sf_bool passcode_in_password;
    sf_bool insecure_mode;
    sf_bool autocommit;
    char *timezone;

    /* used when updating parameters */
    pthread_mutex_t mutex_parameters;

    char *authenticator;

    // Overrider application name and version
    char *application_name;
    char *application_version;

    // Session info
    char *token;
    char *master_token;

    int64 login_timeout;
    int64 network_timeout;

    // Session specific fields
    int64 sequence_counter;
    pthread_mutex_t mutex_sequence_counter;
    char request_id[SF_UUID4_LEN];

    // Error
    SF_ERROR error;
} SF_CONNECT;

/**
 * Column description context
 */
typedef struct sf_snowflake_column_desc {
    char *name;
    SF_TYPE type;
    SF_C_TYPE c_type;
    int64 byte_size;
    int64 internal_size;
    int64 precision;
    int64 scale;
    sf_bool null_ok;
} SF_COLUMN_DESC;

/**
 * Chunk downloader context
 */
typedef struct sf_chunk_downloader SF_CHUNK_DOWNLOADER;

/**
 * Statement context
 */
typedef struct sf_snowflake_statement {
    /* TODO */
    char sfqid[SF_UUID4_LEN];
    int64 sequence_counter;
    char request_id[SF_UUID4_LEN];
    SF_ERROR error;
    SF_CONNECT *connection;
    char *sql_text;
    void *raw_results;
    int64 chunk_rowcount;
    int64 total_rowcount;
    int64 total_fieldcount;
    int64 total_row_index;
    void *params;
    void *results;
    SF_COLUMN_DESC *desc;
    void *stmt_attrs;
    sf_bool is_dml;
    SF_CHUNK_DOWNLOADER *chunk_downloader;
} SF_STMT;

/**
 * Bind input parameter context
 */
typedef struct sf_snowflake_input {
    size_t idx; /* One based index of the columns */
    SF_C_TYPE c_type; /* input data type in C */
    void *value; /* input value */
    size_t len; /* input value length. valid only for SF_C_TYPE_STRING */
    SF_TYPE type; /* (optional) target Snowflake data type */
} SF_BIND_INPUT;

/**
 * Bind output parameter context
 */
typedef struct sf_snowflake_output {
    size_t idx; /* One based index of the columns */
    SF_C_TYPE c_type; /* expected data type in C */
    size_t max_length; /* maximum buffer size provided by application */
    void *value; /* input and output: buffer to stoare a value */
    size_t len; /* output: actual value length */
    sf_bool is_null; /* output: SF_BOOLEAN_TRUE if is null else SF_BOOLEAN_FALSE */
} SF_BIND_OUTPUT;

/**
 * Constants
 */


/**
 * Global Snowflake initialization.
 *
 * @return 0 if successful, errno otherwise
 */
SF_STATUS STDCALL snowflake_global_init(const char *log_path);

/**
 * Global Snowflake cleanup.
 *
 * @return 0 if successful, errno otherwise
 */
SF_STATUS STDCALL snowflake_global_term();

/**
 * Set a global attribute
 * @param type
 * @param value
 * @return
 */
SF_STATUS STDCALL snowflake_global_set_attribute(
  SF_GLOBAL_ATTRIBUTE type, const void *value);

// TODO set description
SF_STATUS STDCALL snowflake_global_get_attribute(
  SF_GLOBAL_ATTRIBUTE type, void *value);

/**
 * Initializes a SNOWFLAKE connection context
 *
 * @return SNOWFLAKE context if success
 */
SF_CONNECT *STDCALL snowflake_init();

/**
 * Purge a SNOWFLAKE connection context
 *
 * @param sf SNOWFLAKE context. The data will be freed from memory.
 * @return 0 if success, otherwise an errno is returned.
 */
SF_STATUS STDCALL snowflake_term(SF_CONNECT *sf);

/**
 * Creates a new session and connects to Snowflake database.
 *
 * @param sf SNOWFLAKE context.
 * @return 0 if success, otherwise an errno is returned.
 */
SF_STATUS STDCALL snowflake_connect(SF_CONNECT *sf);

/**
 * Sets the attribute to the session.
 *
 * @param sf SNOWFLAKE context.
 * @param type the attribute name type
 * @param value pointer to the attribute value
 * @return 0 if success, otherwise an errno is returned.
 */
SF_STATUS STDCALL snowflake_set_attribute(
  SF_CONNECT *sf, SF_ATTRIBUTE type, const void *value);

/**
 * Gets the attribute value from the session.
 *
 * @param sf SNOWFLAKE context.
 * @param type the attribute name type
 * @param value pointer to the attribute value buffer
 * @return 0 if success, otherwise an errno is returned.
 */
SF_STATUS STDCALL snowflake_get_attribute(
  SF_CONNECT *sf, SF_ATTRIBUTE type, void **value);

/**
 * Creates sf SNOWFLAKE_STMT context.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 */
SF_STMT *STDCALL snowflake_stmt(SF_CONNECT *sf);

/**
 * Closes and terminates a statement context
 * @param sfstmt SNOWFLAKE_STMT context.
 * @return 0 if success, otherwise an errno is returned.
 */
void STDCALL snowflake_stmt_term(SF_STMT *sfstmt);

/**
 * Begins a new transaction.
 *
 * @param sf SNOWFLAKE context.
 * @return 0 if success, otherwise an errno is returned.
 */
SF_STATUS STDCALL snowflake_trans_begin(SF_CONNECT *sf);

/**
 * Commits a current transaction.
 *
 * @param sf SNOWFLAKE context.
 * @return 0 if success, otherwise an errno is returned.
 */
SF_STATUS STDCALL snowflake_trans_commit(SF_CONNECT *sf);

/**
 * Rollbacks a current transaction.
 *
 * @param sf SNOWFLAKE context.
 * @return 0 if success, otherwise an errno is returned.
 */
SF_STATUS STDCALL snowflake_trans_rollback(SF_CONNECT *sf);

/**
 * Returns an error context for the SNOWFLAKE_STMT context.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @return error context
 */
SF_ERROR *STDCALL snowflake_stmt_error(SF_STMT *sfstmt);

/**
 * Returns an error context for the SNOWFLAKE context.
 *
 * @param sf SNOWFLAKE context.
 * @return error context
 */
SF_ERROR *STDCALL snowflake_error(SF_CONNECT *sf);

/**
 * Propagate SF_STMT error to SF_CONNECT so that the latest statement
 * error is visible in the connection context.
 *
 * @param sf SNOWFLAKE context
 * @param sfstmt SNOWFLAKE_STMT context.
 * @return 0 if success, otherwise an errno is returned.
 */
SF_STATUS STDCALL snowflake_propagate_error(SF_CONNECT *sf, SF_STMT *sfstmt);

/**
 * Executes a query and returns result set. This function works only for
 * queries and commands that return result set. If no result set is returned,
 * NULL is returned.
 *
 * @param sf SNOWFLAKE_STMT context.
 * @param command a query or command that returns results.
 * @return 0 if success, otherwise an errno is returned.
 */
SF_STATUS STDCALL
snowflake_query(SF_STMT *sfstmt, const char *command, size_t command_size);

/**
 * Returns the number of affected rows in the last execution.  This function
 * works only for DML, i.e., INSERT, UPDATE, DELETE, MULTI TABLE INSERT, MERGE
 * and COPY.
 *
 * @param sf SNOWFLAKE_STMT context.
 * @return the number of affected rows
 */
int64 STDCALL snowflake_affected_rows(SF_STMT *sfstmt);

/**
 * Returns the number of rows can be fetched from the result set.
 *
 * @param sfstmt SNOWFLAKE_RESULTSET context.
 * @return the number of rows.
 */
uint64 STDCALL snowflake_num_rows(SF_STMT *sfstmt);

/**
 * Returns the number of fields in the result set.
 *
 * @param sfstmt SNOWFLAKE_RESULTSET context.
 * @return the number of fields.
 */
uint64 STDCALL snowflake_num_fields(SF_STMT *sfstmt);

/**
 * Returns a SQLState for the result set.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @return SQL State
 */
const char *STDCALL snowflake_sqlstate(SF_STMT *sfstmt);

/**
 * Gets an array of column metadata. The value returned by snowflake_num_fields is the size of the column metadata array
 *
 * @param sf SNOWFLAKE_STMT context.
 * @return SF_COLUMN_DESC if success or NULL
 */
SF_COLUMN_DESC *STDCALL snowflake_desc(SF_STMT *sfstmt);

/**
 * Prepares a statement.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @param command a query or command that returns results.
 * @return 0 if success, otherwise an errno is returned.
 */
SF_STATUS STDCALL
snowflake_prepare(SF_STMT *sfstmt, const char *command, size_t command_size);

/**
 * Sets a statement attribute.
 *
 * @param sf SNOWFLAKE_STMT context.
 * @param type the attribute name type
 * @param value pointer to the attribute value
 * @return 0 if success, otherwise an errno is returned.
 */
SF_STATUS STDCALL
snowflake_stmt_set_attr(SF_STMT *sfstmt, SF_STMT_ATTRIBUTE type,
                        const void *value);

/**
 * Gets a statement attribute value.
 *
 * @param sf SNOWFLAKE_STMT context.
 * @param type the attribute name type
 * @param value pointer to the attribute value buffer
 * @return 0 if success, otherwise an errno is returned.
 */
SF_STATUS STDCALL
snowflake_stmt_get_attr(SF_STMT *sfstmt, SF_STMT_ATTRIBUTE type, void *value);

/**
 * Executes a statement.
 * @param sfstmt SNOWFLAKE_STMT context.
 *
 * @return 0 if success, otherwise an errno is returned.
 */
SF_STATUS STDCALL snowflake_execute(SF_STMT *sfstmt);

/**
 * Fetches the next row for the statement and stores on the bound buffer
 * if any. Noop if no buffer is bound.
 *
 * @param sfstmt SNOWFLAKE_RESULTSET context.
 * @return 0 if success, otherwise an errno is returned.
 */
SF_STATUS STDCALL snowflake_fetch(SF_STMT *sfstmt);

/**
 * Returns the number of binding parameters in the statement.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @return the number of binding parameters in the statement.
 */
uint64 STDCALL snowflake_num_params(SF_STMT *sfstmt);

/**
 * Binds parameters with the statement for execution.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @param sfbind SNOWFLAKE_BIND_INPUT context array.
 * @return 0 if success, otherwise an errno is returned.
 */
SF_STATUS STDCALL snowflake_bind_param(
  SF_STMT *sfstmt, SF_BIND_INPUT *sfbind);

/**
 * Binds an array of parameters with the statement for execution.
 *
 * @param sfstmt SF_STMT context.
 * @param sfbind_array SF_BIND_INPUT array of bind input values.
 * @param size size_t size of the parameter array (sfbind_array).
 * @return 0 if success, otherwise an errno is returned.
 */
SF_STATUS snowflake_bind_param_array(
  SF_STMT *sfstmt, SF_BIND_INPUT *sfbind_array, size_t size);

/**
 * Binds buffers with the statement for result set processing.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @param sfbind_array SNOWFLAKE_BIND_OUTPUT context array.
 * @return 0 if success, otherwise an errno is returned.
 */
SF_STATUS STDCALL snowflake_bind_result(
  SF_STMT *sfstmt, SF_BIND_OUTPUT *sfbind);

/**
 * Binds an array of buffers with the statement for result set processing.
 *
 * @param sfstmt SF_STMT context.
 * @param sfbind_array SF_BIND_OUTPUT array of result buffers.
 * @param size size_t size of the result buffer array (sfbind_array).
 * @return 0 if success, otherwise an errno is returned.
 */
SF_STATUS snowflake_bind_result_array(
  SF_STMT *sfstmt, SF_BIND_OUTPUT *sfbind_array, size_t size);

/**
 * Returns a query id associated with the statement after execution. If not
 * executed, NULL is returned.
 *
 * @param sfstmt SNOWFLAKE_STMT context.
 * @return query id associated with the statement.
 */
const char *STDCALL snowflake_sfqid(SF_STMT *sfstmt);

/**
 * Converts Snowflake Type enum value to a string representation
 * @param type SF_TYPE enum
 * @return a string representation of Snowflake Type
 */
const char *STDCALL snowflake_type_to_string(SF_TYPE type);

/**
 * Converts Snowflake C Type enum value to a string representation
 * @param type SF_C_TYPE
 * @return a string representation of Snowflake C Type
 */
const char *STDCALL snowflake_c_type_to_string(SF_C_TYPE type);

#ifdef  __cplusplus
}
#endif

#endif //SNOWFLAKE_CLIENT_H
