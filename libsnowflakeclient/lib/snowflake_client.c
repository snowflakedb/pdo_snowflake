/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <snowflake_client.h>
#include <openssl/crypto.h>
#include "constants.h"
#include "snowflake_client_int.h"
#include "connection.h"
#include "snowflake_memory.h"
#include "log.h"
#include "results.h"
#include "error.h"

#define curl_easier_escape(curl, string) curl_easy_escape(curl, string, 0)

// Define internal constants
sf_bool DISABLE_VERIFY_PEER;
char *CA_BUNDLE_FILE;
int32 SSL_VERSION;
sf_bool DEBUG;


static char *LOG_PATH = NULL;
static FILE *LOG_FP = NULL;

/*
 * Convenience method to find string size, create buffer, copy over, and return.
 */
void alloc_buffer_and_copy(char **var, const char *str) {
    size_t str_size;
    SF_FREE(*var);
    // If passed in string is null, then return since *var is already null from being freed
    if (str) {
        str_size = strlen(str) + 1; // For null terminator
        *var = (char *) SF_CALLOC(1, str_size);
        strncpy(*var, str, str_size);
    }
}

int mkpath(char* file_path, mode_t mode) {
    assert(file_path && *file_path);
    char* p;
    for (p=strchr(file_path+1, '/'); p; p=strchr(p+1, '/')) {
        *p='\0';
        if (mkdir(file_path, mode)==-1) {
            if (errno!=EEXIST) { *p='/'; return -1; }
        }
        *p='/';
    }
    return 0;
}

/*
 * Initializes logging file
 */
sf_bool STDCALL log_init() {
    sf_bool ret = SF_BOOLEAN_FALSE;
    time_t current_time;
    struct tm * time_info;
    char time_str[15];
    time(&current_time);
    time_info = localtime(&current_time);
    strftime(time_str, sizeof(time_str), "%Y%m%d%H%M%S", time_info);

    size_t log_path_size = 1; //Start with 1 to include null terminator
    log_path_size += strlen(time_str);
    char *sf_log_path = getenv("SNOWFLAKE_LOG_PATH");
    // Set logging level
    if (DEBUG) {
        log_set_quiet(SF_BOOLEAN_FALSE);
    } else {
        log_set_quiet(SF_BOOLEAN_TRUE);
    }
    log_set_level(LOG_TRACE);
    // If log path is specified, use absolute path. Otherwise set logging dir to be relative to current directory
    if (sf_log_path) {
        log_path_size += strlen(sf_log_path);
        log_path_size += 16; // Size of static format characters
        LOG_PATH = (char *) SF_CALLOC(1, log_path_size);
        snprintf(LOG_PATH, log_path_size, "%s/.capi/logs/%s.txt", sf_log_path, (char *)time_str);
    } else {
        log_path_size += 9; // Size of static format characters
        LOG_PATH = (char *) SF_CALLOC(1, log_path_size);
        snprintf(LOG_PATH, log_path_size, "logs/%s.txt", (char *)time_str);
    }
    if (LOG_PATH != NULL) {
        // Create log file path (if it already doesn't exist)
        if (mkpath(LOG_PATH, 0755) == -1) {
            fprintf(stderr, "Error creating log directory. Error code: %s\n", strerror(errno));
            goto cleanup;
        }
        // Open log file
        LOG_FP = fopen(LOG_PATH, "w+");
        if (LOG_FP) {
            // Set log file
            log_set_fp(LOG_FP);
        } else {
            fprintf(stderr, "Error opening file from file path: %s\nError code: %s\n", LOG_PATH, strerror(errno));
            goto cleanup;
        }

    } else {
        fprintf(stderr, "Log path is NULL. Was there an error during path construction?\n");
        goto cleanup;
    }

    ret = SF_BOOLEAN_TRUE;

cleanup:
    return ret;
}

/*
 * Cleans up memory allocated for log init and closes log file.
 */
void STDCALL log_term() {
    SF_FREE(LOG_PATH);
    if (LOG_FP) {
        fclose(LOG_FP);
        log_set_fp(NULL);
    }
}

SNOWFLAKE_STATUS STDCALL snowflake_global_init() {
    SNOWFLAKE_STATUS ret = SF_STATUS_ERROR;
    CURLcode curl_ret;

    // Initialize constants
    DISABLE_VERIFY_PEER = SF_BOOLEAN_FALSE;
    CA_BUNDLE_FILE = NULL;
    SSL_VERSION = CURL_SSLVERSION_TLSv1_2;
    DEBUG = SF_BOOLEAN_FALSE;

    // TODO Add log init error handling
    if (!log_init()) {
        log_fatal("Error during log initialization");
        goto cleanup;
    }
    curl_ret = curl_global_init(CURL_GLOBAL_DEFAULT);
    if(curl_ret != CURLE_OK) {
        log_fatal("curl_global_init() failed: %s", curl_easy_strerror(curl_ret));
        goto cleanup;
    }

    ret = SF_STATUS_SUCCESS;

cleanup:
    return ret;
}

SNOWFLAKE_STATUS STDCALL snowflake_global_term() {
    log_term();
    curl_global_cleanup();

    // Cleanup Constants
    SF_FREE(CA_BUNDLE_FILE);

    sf_alloc_map_to_log(SF_BOOLEAN_TRUE);
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_global_set_attribute(SNOWFLAKE_GLOBAL_ATTRIBUTE type, const void *value) {
    switch (type) {
        case SF_GLOBAL_DISABLE_VERIFY_PEER:
            DISABLE_VERIFY_PEER = *(sf_bool *) value;
            break;
        case SF_GLOBAL_CA_BUNDLE_FILE:
            alloc_buffer_and_copy(&CA_BUNDLE_FILE, value);
            break;
        case SF_GLOBAL_SSL_VERSION:
            SSL_VERSION = *(int32 *) value;
            break;
        case SF_GLOBAL_DEBUG:
            DEBUG = *(sf_bool *) value;
            if (DEBUG) {
                log_set_quiet(SF_BOOLEAN_FALSE);
                log_set_level(LOG_TRACE);
            } else {
                log_set_quiet(SF_BOOLEAN_TRUE);
                log_set_level(LOG_INFO);
            }
            break;
        default:
            break;
    }
}

SNOWFLAKE *STDCALL snowflake_init() {
    SNOWFLAKE *sf = (SNOWFLAKE *) SF_CALLOC(1, sizeof(SNOWFLAKE));

    // Make sure memory was actually allocated
    if (sf) {
        // Initialize object with default values
        sf->host = NULL;
        sf->port = NULL;
        sf->user = NULL;
        sf->password = NULL;
        sf->database = NULL;
        sf->account = NULL;
        sf->role = NULL;
        sf->warehouse = NULL;
        sf->schema = NULL;
        alloc_buffer_and_copy(&sf->protocol, "https");
        sf->passcode = NULL;
        sf->passcode_in_password = SF_BOOLEAN_FALSE;
        sf->insecure_mode = SF_BOOLEAN_FALSE;
        sf->autocommit = SF_BOOLEAN_FALSE;
        sf->token = NULL;
        sf->master_token = NULL;
        sf->login_timeout = 120;
        sf->network_timeout = 0;
        sf->sequence_counter = 0;
        uuid4_generate(sf->request_id);
        clear_snowflake_error(&sf->error);
    }

    return sf;
}

void STDCALL snowflake_term(SNOWFLAKE *sf) {
    // Ensure object is not null
    if (sf) {
        SF_FREE(sf->host);
        SF_FREE(sf->port);
        SF_FREE(sf->user);
        SF_FREE(sf->password);
        SF_FREE(sf->database);
        SF_FREE(sf->account);
        SF_FREE(sf->role);
        SF_FREE(sf->warehouse);
        SF_FREE(sf->schema);
        SF_FREE(sf->protocol);
        SF_FREE(sf->passcode);
        SF_FREE(sf->master_token);
        SF_FREE(sf->token);
    }
    SF_FREE(sf);
}

SNOWFLAKE_STATUS STDCALL snowflake_connect(SNOWFLAKE *sf) {
    if (!sf) {
        return SF_STATUS_ERROR;
    }
    // Reset error context
    clear_snowflake_error(&sf->error);
    cJSON *body = NULL;
    cJSON *data = NULL;
    cJSON *resp = NULL;
    char *s_body = NULL;
    char *s_resp = NULL;
    // Encoded URL to use with libcurl
    URL_KEY_VALUE url_params[] = {
            {"request_id=", sf->request_id, NULL, NULL, 0, 0},
            {"&databaseName=", sf->database, NULL, NULL, 0, 0},
            {"&schemaName=", sf->schema, NULL, NULL, 0, 0},
            {"&warehouse=", sf->warehouse, NULL, NULL, 0, 0},
            {"&roleName=", sf->role, NULL, NULL, 0, 0},
    };
    SNOWFLAKE_STATUS ret = SF_STATUS_ERROR;

    if(is_string_empty(sf->user) || is_string_empty(sf->account)) {
        // Invalid connection
        log_error("Missing essential connection parameters. Either user or account (or both) are missing");
        SET_SNOWFLAKE_ERROR(&sf->error,
                            SF_ERROR_BAD_CONNECTION_PARAMS,
                            "Missing essential connection parameters. Either user or account (or both) are missing",
                            "");
        goto cleanup;
    }

    // Create body
    body = create_auth_json_body(sf, "C API", "C API", "0.1");
    log_debug("Created body");
    s_body = cJSON_Print(body);
    // TODO delete password before printing
    if (DEBUG) {
        log_trace("Here is constructed body:\n%s", s_body);
    }

    // Send request and get data
    if (request(sf, &resp, SESSION_URL, url_params, 5, s_body, NULL, POST_REQUEST_TYPE, &sf->error)) {
        s_resp = cJSON_Print(resp);
        log_trace("Here is JSON response:\n%s", s_resp);
        data = cJSON_GetObjectItem(resp, "data");
        // Get token
        if (json_copy_string(&sf->token, data, "token")) {
            log_error("No valid token found in response");
            SET_SNOWFLAKE_ERROR(&sf->error, SF_ERROR_BAD_JSON, "Cannot find valid session token in response", "");
            goto cleanup;
        }
        // Get master token
        if (json_copy_string(&sf->master_token, data, "masterToken")) {
            log_error("No valid master token found in response");
            SET_SNOWFLAKE_ERROR(&sf->error, SF_ERROR_BAD_JSON, "Cannot find valid master token in response", "");
            goto cleanup;
        }
    } else {
        log_error("No response");
        SET_SNOWFLAKE_ERROR(&sf->error, SF_ERROR_BAD_JSON, "No valid JSON response", "");
        goto cleanup;
    }

    /* we are done... */
    ret = SF_STATUS_SUCCESS;

cleanup:
    // Delete password and passcode for security's sake
    if (sf->password) {
        // Write over password in memory including null terminator
        memset(sf->password, 0, strlen(sf->password) + 1);
        SF_FREE(sf->password);
    }
    if (sf->passcode) {
        // Write over passcode in memory including null terminator
        memset(sf->passcode, 0, strlen(sf->passcode) + 1);
        SF_FREE(sf->passcode);
    }
    cJSON_Delete(body);
    cJSON_Delete(resp);
    SF_FREE(s_body);
    SF_FREE(s_resp);

    return ret;
}

SNOWFLAKE_STATUS STDCALL snowflake_close(SNOWFLAKE *sf) {
    if (!sf) {
        return SF_STATUS_ERROR;
    }
    clear_snowflake_error(&sf->error);
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_set_attr(
        SNOWFLAKE *sf, SNOWFLAKE_ATTRIBUTE type, const void *value) {
    if (!sf) {
        return SF_STATUS_ERROR;
    }
    clear_snowflake_error(&sf->error);
    switch (type) {
        case SF_CON_ACCOUNT:
            alloc_buffer_and_copy(&sf->account, value);
            break;
        case SF_CON_USER:
            alloc_buffer_and_copy(&sf->user, value);
            break;
        case SF_CON_PASSWORD:
            alloc_buffer_and_copy(&sf->password, value);
            break;
        case SF_CON_DATABASE:
            alloc_buffer_and_copy(&sf->database, value);
            break;
        case SF_CON_SCHEMA:
            alloc_buffer_and_copy(&sf->schema, value);
            break;
        case SF_CON_WAREHOUSE:
            alloc_buffer_and_copy(&sf->warehouse, value);
            break;
        case SF_CON_ROLE:
            alloc_buffer_and_copy(&sf->role, value);
            break;
        case SF_CON_HOST:
            alloc_buffer_and_copy(&sf->host, value);
            break;
        case SF_CON_PORT:
            alloc_buffer_and_copy(&sf->port, value);
            break;
        case SF_CON_PROTOCOL:
            alloc_buffer_and_copy(&sf->protocol, value);
            break;
        case SF_CON_PASSCODE:
            alloc_buffer_and_copy(&sf->passcode, value);
            break;
        case SF_CON_PASSCODE_IN_PASSWORD:
            sf->passcode_in_password = *((sf_bool *) value);
            break;
        case SF_CON_APPLICATION:
            // TODO Implement this
            break;
        case SF_CON_AUTHENTICATOR:
            // TODO Implement this
            break;
        case SF_CON_INSECURE_MODE:
            sf->insecure_mode = *((sf_bool *) value);
            break;
        case SF_SESSION_PARAMETER:
            // TODO Implement this
            break;
        case SF_CON_LOGIN_TIMEOUT:
            sf->login_timeout = *((int64 *) value);
            break;
        case SF_CON_NETWORK_TIMEOUT:
            sf->network_timeout = *((int64 *) value);
            break;
        case SF_CON_AUTOCOMMIT:
            sf->autocommit = *((sf_bool *) value);
            break;
        default:
            SET_SNOWFLAKE_ERROR(&sf->error, SF_ERROR_BAD_ATTRIBUTE_TYPE, "Invalid attribute type", "");
            return SF_STATUS_ERROR;
    }
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_get_attr(
        SNOWFLAKE *sf, SNOWFLAKE_ATTRIBUTE type, void **value) {
    if (!sf) {
        return SF_STATUS_ERROR;
    }
    clear_snowflake_error(&sf->error);
    //TODO Implement this
}

/**
 * Resets SNOWFLAKE_STMT parameters.
 *
 * @param sfstmt
 * @param free free allocated memory if true
 */
static void STDCALL _snowflake_stmt_reset(SNOWFLAKE_STMT *sfstmt) {
    int64 i;
    clear_snowflake_error(&sfstmt->error);

    strncpy(sfstmt->sfqid, "", UUID4_LEN);
    strncpy(sfstmt->sqlstate, "", SQLSTATE_LEN);
    uuid4_generate(sfstmt->request_id);

    if (sfstmt->sql_text) {
        SF_FREE(sfstmt->sql_text); /* SQL */
    }
    sfstmt->sql_text = NULL;

    if (sfstmt->raw_results) {
        cJSON_Delete(sfstmt->raw_results);
    }
    sfstmt->raw_results = NULL;

    if (sfstmt->params) {
        array_list_deallocate(sfstmt->params); /* binding parameters */
    }
    sfstmt->params = NULL;

    if (sfstmt->results) {
        array_list_deallocate(sfstmt->results); /* binding columns */
    }
    sfstmt->results = NULL;

    if (sfstmt->desc) {
        /* column metadata */
        for (i = 0; i < sfstmt->total_fieldcount; i++) {
            SF_FREE(sfstmt->desc[i]->name);
            SF_FREE(sfstmt->desc[i]);
        }
        SF_FREE(sfstmt->desc);
    }
    sfstmt->desc = NULL;

    sfstmt->total_rowcount = -1;
    sfstmt->total_fieldcount = -1;
    sfstmt->total_row_index = -1;
}

SNOWFLAKE_STMT *STDCALL snowflake_stmt(SNOWFLAKE *sf) {
    if (!sf) {
        return NULL;
    }

    SNOWFLAKE_STMT *sfstmt = (SNOWFLAKE_STMT *) SF_CALLOC(1, sizeof(SNOWFLAKE_STMT));
    if (sfstmt) {
        _snowflake_stmt_reset(sfstmt);
        sfstmt->sequence_counter = ++sf->sequence_counter;
        sfstmt->connection = sf;
    }
    return sfstmt;
}

void STDCALL snowflake_stmt_close(SNOWFLAKE_STMT *sfstmt) {
    if (sfstmt) {
        _snowflake_stmt_reset(sfstmt);
        SF_FREE(sfstmt);
    }
}

SNOWFLAKE_STATUS STDCALL snowflake_bind_param(
    SNOWFLAKE_STMT *sfstmt, SNOWFLAKE_BIND_INPUT *sfbind) {
    if (!sfstmt) {
        return SF_STATUS_ERROR;
    }
    clear_snowflake_error(&sfstmt->error);
    if (sfstmt->params == NULL) {
        sfstmt->params = array_list_init();
    }
    array_list_set(sfstmt->params, sfbind, sfbind->idx);
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_bind_result(
    SNOWFLAKE_STMT *sfstmt, SNOWFLAKE_BIND_OUTPUT *sfbind) {
    if (!sfstmt) {
        return SF_STATUS_ERROR;
    }
    clear_snowflake_error(&sfstmt->error);
    if (sfstmt->results == NULL) {
        sfstmt->results = array_list_init();
    }
    array_list_set(sfstmt->results, sfbind, sfbind->idx);
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_query(
        SNOWFLAKE_STMT *sfstmt, const char *command) {
    if (!sfstmt) {
        return SF_STATUS_ERROR;
    }
    clear_snowflake_error(&sfstmt->error);
    if (snowflake_prepare(sfstmt, command) != SF_STATUS_SUCCESS) {
        return SF_STATUS_ERROR;
    }
    if (snowflake_execute(sfstmt) != SF_STATUS_SUCCESS) {
        return SF_STATUS_ERROR;
    }
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_fetch(SNOWFLAKE_STMT *sfstmt) {
    if (!sfstmt) {
        return SF_STATUS_ERROR;
    }
    clear_snowflake_error(&sfstmt->error);
    SNOWFLAKE_STATUS ret = SF_STATUS_ERROR;
    int64 i;
    cJSON *row = NULL;
    cJSON *raw_result;
    SNOWFLAKE_BIND_OUTPUT *result;

    // If no more results, set return to SF_STATUS_EOL
    if (cJSON_GetArraySize(sfstmt->raw_results) == 0) {
        ret = SF_STATUS_EOL;
        goto cleanup;
    }

    // Check that we can write to the provided result bindings
    for (i = 0; i < sfstmt->total_fieldcount; i++) {
        result = array_list_get(sfstmt->results, i + 1);
        if (result == NULL) {
            continue;
        } else {
            if (result->type != sfstmt->desc[i]->c_type && result->type != SF_C_TYPE_STRING) {
                // TODO add error msg
                goto cleanup;
            }
        }
    }

    // Get next result row
    row = cJSON_DetachItemFromArray(sfstmt->raw_results, 0);

    // Write to results
    // TODO error checking for conversions during fetch
    for (i = 0; i < sfstmt->total_fieldcount; i++) {
        result = array_list_get(sfstmt->results, i + 1);
        if (result == NULL) {
            continue;
        } else {
            raw_result = cJSON_GetArrayItem(row, i);
            // TODO turn into switch statement
            if (result->type == SF_C_TYPE_INT8) {
                if (sfstmt->desc[i]->type == SF_TYPE_BOOLEAN) {
                    *(int8 *) result->value = cJSON_IsTrue(raw_result) ? SF_BOOLEAN_TRUE : SF_BOOLEAN_FALSE;
                } else {
                    // field is a char?
                    *(int8 *) result->value = (int8) raw_result->valuestring[0];
                }
            } else if (result->type == SF_C_TYPE_UINT8) {
                *(uint8 *) result->value = (uint8) raw_result->valuestring[0];
            } else if (result->type == SF_C_TYPE_INT64) {
                *(int64 *) result->value = (int64) strtoll(raw_result->valuestring, NULL, 10);
            } else if (result->type == SF_C_TYPE_UINT64) {
                *(uint64 *) result->value = (uint64) strtoull(raw_result->valuestring, NULL, 10);
            } else if (result->type == SF_C_TYPE_FLOAT64) {
                *(float64 *) result->value = (float64) strtod(raw_result->valuestring, NULL);
            } else if (result->type == SF_C_TYPE_STRING) {
                /* copy original data as is except Date/Time/Timestamp/Binary type */
                strncpy(result->value, raw_result->valuestring, result->max_length);
            } else if (result->type == SF_C_TYPE_TIMESTAMP) {
                // TODO Do some timestamp stuff here
            } else {
                // TODO Create default case
            }
        }
    }

    ret = SF_STATUS_SUCCESS;

cleanup:
    cJSON_Delete(row);
    return ret;
}

SNOWFLAKE_STATUS STDCALL snowflake_trans_begin(SNOWFLAKE *sf) {
    if (!sf) {
        return SF_STATUS_ERROR;
    }
    clear_snowflake_error(&sf->error);
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_trans_commit(SNOWFLAKE *sf) {
    if (!sf) {
        return SF_STATUS_ERROR;
    }
    clear_snowflake_error(&sf->error);
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_trans_rollback(SNOWFLAKE *sf) {
    if (!sf) {
        return SF_STATUS_ERROR;
    }
    clear_snowflake_error(&sf->error);
    return SF_STATUS_SUCCESS;
}

int64 STDCALL snowflake_affected_rows(SNOWFLAKE_STMT *sfstmt) {
    if (!sfstmt) {
        return -1;
    }
    return 0;
}

SNOWFLAKE_STATUS STDCALL snowflake_prepare(SNOWFLAKE_STMT *sfstmt, const char *command) {
    if (!sfstmt) {
        return SF_STATUS_ERROR;
    }
    clear_snowflake_error(&sfstmt->error);
    SNOWFLAKE_STATUS ret = SF_STATUS_ERROR;
    size_t sql_text_size = 1; // Don't forget about null terminator
    if (!command) {
        goto cleanup;
    }
    _snowflake_stmt_reset(sfstmt);
    // Set sql_text to command
    sql_text_size += strlen(command);
    sfstmt->sql_text = (char *) SF_CALLOC(1, sql_text_size);
    strncpy(sfstmt->sql_text, command, sql_text_size);

    ret = SF_STATUS_SUCCESS;

cleanup:
    return ret;
}

SNOWFLAKE_STATUS STDCALL snowflake_execute(SNOWFLAKE_STMT *sfstmt) {
    if (!sfstmt) {
        return SF_STATUS_ERROR;
    }
    clear_snowflake_error(&sfstmt->error);
    SNOWFLAKE_STATUS ret = SF_STATUS_ERROR;
    SNOWFLAKE_JSON_ERROR json_error;
    const char *error_msg;
    cJSON *body = NULL;
    cJSON *data = NULL;
    cJSON *rowtype = NULL;
    cJSON *resp = NULL;
    char *s_body = NULL;
    char *s_resp = NULL;
    sf_bool success = SF_BOOLEAN_FALSE;
    URL_KEY_VALUE url_params[] = {
            {"requestId=", sfstmt->request_id, NULL, NULL, 0, 0}
    };
    size_t i;
    cJSON *bindings = NULL;
    SNOWFLAKE_BIND_INPUT *input;
    const char *type;
    char *value;

    // TODO Do error handing and checking and stuff
    // TODO move this to execute function
    if (sfstmt->params && sfstmt->params->used > 0) {
        bindings = cJSON_CreateObject();
        for (i = 0; i < sfstmt->params->used; i++) {
            cJSON *binding;
            input = (SNOWFLAKE_BIND_INPUT *) array_list_get(sfstmt->params, i + 1);
            // TODO check if input is null and either set error or write msg to log
            type = snowflake_type_to_string(c_type_to_snowflake(input->c_type, SF_TYPE_TIMESTAMP_NTZ));
            value = value_to_string(input->value, input->c_type);
            binding = cJSON_CreateObject();
            char idxbuf[20];
            sprintf(idxbuf, "%ld", i + 1);
            cJSON_AddStringToObject(binding, "type", type);
            cJSON_AddStringToObject(binding, "value", value);
            cJSON_AddItemToObject(bindings, idxbuf, binding);
            SF_FREE(value);
        }
    }

    if (is_string_empty(sfstmt->connection->master_token) || is_string_empty(sfstmt->connection->token)) {
        log_error("Missing session token or Master token. Are you sure that snowflake_connect was successful?");
        SET_SNOWFLAKE_ERROR(&sfstmt->error, SF_ERROR_BAD_CONNECTION_PARAMS,
                            "Missing session or master token. Try running snowflake_connect.", "");
        goto cleanup;
    }

    // Create Body
    body = create_query_json_body(sfstmt->sql_text, sfstmt->sequence_counter);
    if (bindings != NULL) {
        /* binding parameters if exists */
        cJSON_AddItemToObject(body, "bindings", bindings);
    }
    s_body = cJSON_Print(body);
    log_debug("Created body");
    log_trace("Here is constructed body:\n%s", s_body);

    if (request(sfstmt->connection, &resp, QUERY_URL, url_params, 1, s_body, NULL, POST_REQUEST_TYPE, &sfstmt->error)) {
        s_resp = cJSON_Print(resp);
        log_trace("Here is JSON response:\n%s", s_resp);
        data = cJSON_GetObjectItem(resp, "data");
        if (json_copy_string_no_alloc(sfstmt->sfqid, data, "queryId", UUID4_LEN)) {
            log_debug("No valid sfqid found in response");
        }
        if (json_copy_string_no_alloc(sfstmt->sqlstate, data, "sqlState", SQLSTATE_LEN)) {
            log_debug("No valid sqlstate found in response");
        }
        if ((json_error = json_copy_bool(&success, resp, "success")) == SF_JSON_NO_ERROR && success) {
            // Set Database info
            if (json_copy_string(&sfstmt->connection->database, data, "finalDatabaseName")) {
                log_warn("No valid database found in response");
            }
            if (json_copy_string(&sfstmt->connection->schema, data, "finalSchemaName")) {
                log_warn("No valid schema found in response");
            }
            if (json_copy_string(&sfstmt->connection->warehouse, data, "finalWarehouseName")) {
                log_warn("No valid warehouse found in response");
            }
            if (json_copy_string(&sfstmt->connection->role, data, "finalRoleName")) {
                log_warn("No valid role found in response");
            }
            rowtype = cJSON_GetObjectItem(data, "rowtype");
            if (cJSON_IsArray(rowtype)) {
                sfstmt->total_fieldcount = cJSON_GetArraySize(rowtype);
                sfstmt->desc = set_description(rowtype);
            }
            // Set results array
            if (json_detach_array_from_object(&sfstmt->raw_results, data, "rowset")) {
                log_error("No valid rowset found in response");
                SET_SNOWFLAKE_ERROR(&sfstmt->error, SF_ERROR_BAD_JSON,
                                    "Missing rowset from response. No results found.", sfstmt->sfqid);
                goto cleanup;
            }
            if (json_copy_int(&sfstmt->total_rowcount, data, "total")) {
                log_warn("No total count found in response. Reverting to using array size of results");
                sfstmt->total_rowcount = cJSON_GetArraySize(sfstmt->raw_results);
            }
        } else if (json_error != SF_JSON_NO_ERROR) {
            JSON_ERROR_MSG(json_error, error_msg, "Success code");
            SET_SNOWFLAKE_ERROR(&sfstmt->error, SF_ERROR_BAD_JSON, error_msg, sfstmt->sfqid);
            goto cleanup;
        } else if (!success) {
            SET_SNOWFLAKE_ERROR(&sfstmt->error, SF_ERROR_APPLICATION_ERROR, "Query was not successful", "");
            goto cleanup;
        }
    } else {
        log_trace("Connection failed");
    }

    // Everything went well if we got to this point
    ret = SF_STATUS_SUCCESS;

cleanup:
    cJSON_Delete(body);
    cJSON_Delete(resp);
    SF_FREE(s_body);
    SF_FREE(s_resp);

    return ret;
}

SNOWFLAKE_ERROR *STDCALL snowflake_error(SNOWFLAKE *sf) {
    if (!sf) {
        return NULL;
    }
    return &sf->error;
}

SNOWFLAKE_ERROR *STDCALL snowflake_stmt_error(SNOWFLAKE_STMT *sfstmt) {
    if (!sfstmt) {
        return NULL;
    }
    return &sfstmt->error;
}

uint64 STDCALL snowflake_num_rows(SNOWFLAKE_STMT *sfstmt) {
    // TODO fix int vs uint stuff
    if (!sfstmt) {
        // TODO change to -1?
        return 0;
    }
    return (uint64)sfstmt->total_rowcount;
}

uint64 STDCALL snowflake_num_fields(SNOWFLAKE_STMT *sfstmt) {
    // TODO fix int vs uint stuff
    if (!sfstmt) {
        // TODO change to -1?
        return 0;
    }
    return (uint64)sfstmt->total_fieldcount;
}

uint64 STDCALL snowflake_param_count(SNOWFLAKE_STMT *sfstmt) {
    if (!sfstmt) {
        // TODO change to -1?
        return 0;
    }
    return sfstmt->params->used;
}

const char *STDCALL snowflake_sfqid(SNOWFLAKE_STMT *sfstmt) {
    if (!sfstmt) {
        return NULL;
    }
    return sfstmt->sfqid;
}

const char *STDCALL snowflake_sqlstate(SNOWFLAKE_STMT *sfstmt) {
    if (!sfstmt) {
        return NULL;
    }
    return sfstmt->sqlstate;
}

SNOWFLAKE_STATUS STDCALL snowflake_stmt_get_attr(
  SNOWFLAKE_STMT *sfstmt, SNOWFLAKE_STMT_ATTRIBUTE type, void *value) {
    if (!sfstmt) {
        return SF_STATUS_ERROR;
    }
    // TODO: get the value from SNOWFLAKE_STMT.
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_stmt_set_attr(
  SNOWFLAKE_STMT *sfstmt, SNOWFLAKE_STMT_ATTRIBUTE type, const void *value) {
    if (!sfstmt) {
        return SF_STATUS_ERROR;
    }
    clear_snowflake_error(&sfstmt->error);
    /* TODO: need extra member in SNOWFLAKE_STMT */
    return SF_STATUS_SUCCESS;
}

