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
#include "snowflake_client_int.h"
#include "constants.h"
#include "connection.h"
#include "snowflake_memory.h"
#include "log.h"
#include "results.h"

#define curl_easier_escape(curl, string) curl_easy_escape(curl, string, 0)

// Define internal constants
sf_bool DISABLE_VERIFY_PEER;
char *CA_BUNDLE_FILE;
int32 SSL_VERSION;


static char *LOG_PATH = NULL;
static FILE *LOG_FP = NULL;

/*
 * Convenience method to find string size, create buffer, copy over, and return.
 */
void alloc_buffer_and_copy(char **var, const char *str) {
    size_t str_size;
    SF_FREE(*var);
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
    struct stat st = {0};
    time_t current_time;
    struct tm * time_info;
    char time_str[15];
    time(&current_time);
    time_info = localtime(&current_time);
    strftime(time_str, sizeof(time_str), "%Y%m%d%H%M%S", time_info);

    size_t log_path_size = 1; //Start with 1 to include null terminator
    log_path_size += strlen(time_str);
    char *sf_log_path = getenv("SNOWFLAKE_LOG_PATH");
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

    // Initialize constants
    DISABLE_VERIFY_PEER = SF_BOOLEAN_FALSE;
    CA_BUNDLE_FILE = NULL;
    SSL_VERSION = CURL_SSLVERSION_TLSv1_2;

    // TODO Create set and get functions for Global constants

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
        default:
            break;
    }
}

SNOWFLAKE *STDCALL snowflake_init() {
    // TODO: track memory usage
    SNOWFLAKE *sf = (SNOWFLAKE *) SF_CALLOC(1, sizeof(SNOWFLAKE));

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

    return sf;
}

void STDCALL snowflake_term(SNOWFLAKE *sf) {
    // TODO: track memory usage
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
    CURL *curl = NULL;
    // Use curl header list
    struct curl_slist *header = NULL;
    cJSON *body = NULL;
    cJSON *data = NULL;
    cJSON *resp = NULL;
    char *s_body = NULL;
    char *s_resp = NULL;
    // Encoded URL to use with libcurl
    char *encoded_url = NULL;
    URL_KEY_VALUE url_params[] = {
            {"request_id=", sf->request_id, NULL, NULL, 0, 0},
            {"&databaseName=", sf->database, NULL, NULL, 0, 0},
            {"&schemaName=", sf->schema, NULL, NULL, 0, 0},
            {"&warehouse=", sf->warehouse, NULL, NULL, 0, 0},
            {"&roleName=", sf->role, NULL, NULL, 0, 0},
    };
    RAW_JSON_BUFFER *raw_json = NULL;
    struct data config;
    config.trace_ascii = 1;
    SNOWFLAKE_STATUS ret = SF_STATUS_ERROR;

    if(!sf->protocol || !sf->host || !sf->port) {
        // Invalid connection
        goto cleanup;
    }

    raw_json = (RAW_JSON_BUFFER *) SF_CALLOC(1, sizeof(RAW_JSON_BUFFER));
    raw_json->size = 0;
    raw_json->buffer = NULL;

    curl = curl_easy_init();

    if (curl) {

        // Create header
        header = create_header_no_token();
        log_info("Created header");

        // Create body
        body = create_auth_json_body(sf, "C API", "C API", "0.1");
        log_info("Created body");
        s_body = cJSON_Print(body);
        log_trace("Here is constructed body:\n%s", s_body);


        // Add up the string lengths and add 5 (4 for the static characters in the encoded url and 1 for the null terminator)
        encoded_url = encode_url(curl, sf->protocol, sf->host, sf->port, SESSION_URL, url_params, 5);

        if (encoded_url == NULL) {
            goto cleanup;
        }

        // Setup curl call
        curl_post_call(&curl, encoded_url, header, s_body, raw_json, &json_resp_cb, &config);
        resp = cJSON_Parse(raw_json->buffer);
        // TODO refactor JSON response
        if (resp) {
            s_resp = cJSON_Print(resp);
            log_trace("Here is JSON response:\n%s", s_resp);
            data = cJSON_GetObjectItem(resp, "data");
            // Get token
            if (!json_copy_string(&sf->token, data, "token")) {
                log_error("No valid token found in response");
            }
            // Get master token
            if (!json_copy_string(&sf->master_token, data, "masterToken")) {
                log_error("No valid master token found in response");
            }
        } else {
            log_error("No response");
            goto cleanup;
        }

        /* we are done... */
        ret = SF_STATUS_SUCCESS;
    }

cleanup:
    curl_slist_free_all(header);
    curl_easy_cleanup(curl);
    cJSON_Delete(body);
    cJSON_Delete(resp);
    SF_FREE(s_body);
    SF_FREE(s_resp);
    if (raw_json) {
        SF_FREE(raw_json->buffer);
    }
    SF_FREE(raw_json);
    SF_FREE(encoded_url);

    return ret;
}

SNOWFLAKE_STATUS STDCALL snowflake_close(SNOWFLAKE *sf) {
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_set_attr(
        SNOWFLAKE *sf, SNOWFLAKE_ATTRIBUTE type, const void *value) {
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
            // TODO Set error
            return SF_STATUS_ERROR;
    }
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_get_attr(
        SNOWFLAKE *sf, SNOWFLAKE_ATTRIBUTE type, void *value) {
    //TODO Implement this
}

SNOWFLAKE_STMT *STDCALL snowflake_stmt(SNOWFLAKE *sf) {
    // TODO: track memory usage
    SNOWFLAKE_STMT *sfstmt = (SNOWFLAKE_STMT *) SF_CALLOC(1, sizeof(SNOWFLAKE_STMT));
    sfstmt->connection = sf;
    sfstmt->sfqid = NULL;
    sfstmt->sqlstate = NULL;
    sfstmt->sequence_counter = ++sf->sequence_counter;
    uuid4_generate(sfstmt->request_id);
    //sfstmt->error = NULL;
    sfstmt->sql_text = NULL;
    sfstmt->raw_results = NULL;
    sfstmt->total_rowcount = -1;
    sfstmt->total_fieldcount = -1;
    sfstmt->total_row_index = -1;
    sfstmt->params = NULL;
    sfstmt->results = NULL;
    sfstmt->desc = NULL;
    return sfstmt;
}

void STDCALL snowflake_stmt_close(SNOWFLAKE_STMT *sfstmt) {
    // TODO: track memory usage
    int64 i;
    if (sfstmt) {
        SF_FREE(sfstmt->sfqid);
        SF_FREE(sfstmt->sqlstate);
        cJSON_Delete(sfstmt->raw_results);
        cJSON_Delete(sfstmt->prepared_inputs);
        SF_FREE(sfstmt->sql_text);
        array_list_deallocate(sfstmt->params);
        array_list_deallocate(sfstmt->results);
        if (sfstmt->desc) {
            for (i = 0; i < sfstmt->total_fieldcount; i++) {
                SF_FREE(sfstmt->desc[i]->name);
                SF_FREE(sfstmt->desc[i]);
            }
        }
        SF_FREE(sfstmt->desc);
    }
    SF_FREE(sfstmt);
}

SNOWFLAKE_STATUS STDCALL snowflake_bind_param(
    SNOWFLAKE_STMT *sfstmt, SNOWFLAKE_BIND_INPUT *sfbind)
{
    if (sfstmt->params == NULL) {
        sfstmt->params = array_list_init();
    }
    array_list_set(sfstmt->params, sfbind, sfbind->idx);
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_bind_result(
    SNOWFLAKE_STMT *sfstmt, SNOWFLAKE_BIND_OUTPUT *sfbind)
{
    if (sfstmt->results == NULL) {
        sfstmt->results = array_list_init();
    }
    array_list_set(sfstmt->results, sfbind, sfbind->idx);
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_query(
        SNOWFLAKE_STMT *sfstmt, const char *command) {
    if (snowflake_prepare(sfstmt, command) != SF_STATUS_SUCCESS) {
        return SF_STATUS_ERROR;
    }
    if (snowflake_execute(sfstmt) != SF_STATUS_SUCCESS) {
        return SF_STATUS_ERROR;
    }
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_fetch(SNOWFLAKE_STMT *sfstmt) {
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
            // TODO do we really need strict parameter checking? We can probably get away with less strict parameter checking
            if (result->type != sfstmt->desc[i]->c_type) {
                // TODO add error msg
                goto cleanup;
            }
        }
    }

    // Get next result row
    row = cJSON_DetachItemFromArray(sfstmt->raw_results, 0);

    // Write to results
    for (i = 0; i < sfstmt->total_fieldcount; i++) {
        result = array_list_get(sfstmt->results, i + 1);
        if (result == NULL) {
            continue;
        } else {
            raw_result = cJSON_GetArrayItem(row, i);
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
                // TODO should we assume that we always have enough space?
                strcpy(result->value, raw_result->valuestring);
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
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_trans_commit(SNOWFLAKE *sf) {
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_trans_rollback(SNOWFLAKE *sf) {
    return SF_STATUS_SUCCESS;
}

uint64 STDCALL snowflake_affected_rows(SNOWFLAKE_STMT *sfstmt) {
    return 0;
}

SNOWFLAKE_STATUS STDCALL snowflake_prepare(SNOWFLAKE_STMT *sfstmt, const char *command) {
    SNOWFLAKE_STATUS ret = SF_STATUS_ERROR;
    size_t sql_text_size = 1; // Don't forget about null terminator
    size_t i;
    cJSON *bindings;
    cJSON *binding;
    SNOWFLAKE_BIND_INPUT *input;
    const char *type;
    char *value;
    if (!command) {
        goto cleanup;
    }
    // Set sql_text to command
    sql_text_size += strlen(command);
    sfstmt->sql_text = (char *) SF_CALLOC(1, sql_text_size);
    strncpy(sfstmt->sql_text, command, sql_text_size);

    // TODO Do error handing and checking and stuff
    if (sfstmt->params && sfstmt->params->used > 0) {
        sfstmt->prepared_inputs = cJSON_CreateObject();
        bindings = cJSON_CreateArray();
        for (i = 0; i < sfstmt->params->size; i++) {
            input = (SNOWFLAKE_BIND_INPUT *) array_list_get(sfstmt->params, i + 1);
            // TODO check if input is null and either set error or write msg to log
            type = snowflake_type_to_string(c_type_to_snowflake(input->c_type, SF_TYPE_TIMESTAMP_NTZ));
            value = value_to_string(input->value, input->c_type);
            binding = cJSON_CreateObject();
            cJSON_AddStringToObject(binding, "type", type);
            cJSON_AddStringToObject(binding, "value", value);
            cJSON_AddItemToArray(bindings, binding);
        }
        cJSON_AddItemToObject(sfstmt->prepared_inputs, "bindings", bindings);
    }

    ret = SF_STATUS_SUCCESS;

cleanup:
    return ret;
}

SNOWFLAKE_STATUS STDCALL snowflake_execute(SNOWFLAKE_STMT *sfstmt) {
    CURL *curl = NULL;
    SNOWFLAKE_STATUS ret = SF_STATUS_ERROR;
    struct curl_slist *header = NULL;
    cJSON *body = NULL;
    cJSON *data = NULL;
    cJSON *rowtype = NULL;
    cJSON *resp = NULL;
    char *s_body = NULL;
    char *s_resp = NULL;
    char *header_token = NULL;
    sf_bool success = SF_BOOLEAN_FALSE;
    char *encoded_url = NULL;
    RAW_JSON_BUFFER *raw_json = NULL;
    struct data config;
    config.trace_ascii = 1;
    size_t header_token_size;
    URL_KEY_VALUE url_params[] = {
            {"requestId=", sfstmt->request_id, NULL, NULL, 0, 0}
    };
    raw_json = (RAW_JSON_BUFFER *) SF_CALLOC(1, sizeof(RAW_JSON_BUFFER));
    raw_json->size = 0;
    raw_json->buffer = NULL;
    if (sfstmt->connection->token == NULL || sfstmt->connection->master_token == NULL) {
        log_error("Missing session token or Master token. Are you sure that snowflake_connect was successful?");
        // TODO Set error statement
        goto cleanup;
    }
    // -2 since there is a placeholder in the token format; +1 for null terminator
    header_token_size = strlen(HEADER_SNOWFLAKE_TOKEN_FORMAT) - 2 + strlen(sfstmt->connection->token) + 1;


    if(!sfstmt->connection->protocol || !sfstmt->connection->host || !sfstmt->connection->port) {
        // TODO Set error statement
        goto cleanup;
    }

    curl = curl_easy_init();

    if (curl) {
        // Create header
        header_token = (char *) SF_CALLOC(1, header_token_size);
        snprintf(header_token, header_token_size, HEADER_SNOWFLAKE_TOKEN_FORMAT, sfstmt->connection->token);
        header = create_header_token(header_token);
        log_info("Created header with token");

        // Create Body
        body = create_query_json_body(sfstmt->sql_text, sfstmt->sequence_counter);
        s_body = cJSON_Print(body);
        log_info("Created body");
        log_trace("Here is constructed body:\n%s", s_body);

        // Encode URL parameters
        encoded_url = encode_url(curl, sfstmt->connection->protocol, sfstmt->connection->host, sfstmt->connection->port, QUERY_URL, url_params, 1);

        if (encoded_url == NULL) {
            goto cleanup;
        }

        /* Check for errors */
        if(!curl_post_call(&curl, encoded_url, header, s_body, raw_json, &json_resp_cb, &config)) {
            goto cleanup;
        }
        resp = cJSON_Parse(raw_json->buffer);
        if (resp) {
            s_resp = cJSON_Print(resp);
            log_trace("Here is JSON response:\n%s", s_resp);
            data = cJSON_GetObjectItem(resp, "data");
            if (!json_copy_string(&sfstmt->sfqid, data, "queryId")) {
                log_error("No valid sfqid found in response");
            }
            if (!json_copy_string(&sfstmt->sqlstate, data, "sqlState")) {
                log_error("No valid sqlstate found in response");
            }
            json_copy_bool(&success, resp, "success");
            if (success) {
                // Set Database info
                if (!json_copy_string(&sfstmt->connection->database, data, "finalDatabaseName")) {
                    log_warn("No valid database found in response");
                }
                if (!json_copy_string(&sfstmt->connection->schema, data, "finalSchemaName")) {
                    log_warn("No valid schema found in response");
                }
                if (!json_copy_string(&sfstmt->connection->warehouse, data, "finalWarehouseName")) {
                    log_warn("No valid warehouse found in response");
                }
                if (!json_copy_string(&sfstmt->connection->role, data, "finalRoleName")) {
                    log_warn("No valid role found in response");
                }
                rowtype = cJSON_GetObjectItem(data, "rowtype");
                if (cJSON_IsArray(rowtype)) {
                    sfstmt->total_fieldcount = cJSON_GetArraySize(rowtype);
                    sfstmt->desc = set_description(rowtype);
                }
                // Set results array
                if (!json_detach_array_from_object(&sfstmt->raw_results, data, "rowset")) {
                    log_error("No valid rowset found in response");
                }
                // TODO get from total field
                sfstmt->total_rowcount = cJSON_GetArraySize(sfstmt->raw_results);
            }
        } else {
            log_trace("Error response:\n%s", raw_json->buffer);
        }
    } else {
        // TODO Add error statement here
        goto cleanup;
    }

    // Everything went well if we got to this point
    ret = SF_STATUS_SUCCESS;

cleanup:
    curl_slist_free_all(header);
    cJSON_Delete(body);
    cJSON_Delete(resp);
    SF_FREE(s_body);
    SF_FREE(s_resp);
    curl_easy_cleanup(curl);
    if (raw_json) {
        SF_FREE(raw_json->buffer);
    }
    SF_FREE(raw_json);
    SF_FREE(header_token);
    SF_FREE(encoded_url);

    return ret;
}

SNOWFLAKE_ERROR *STDCALL snowflake_error(SNOWFLAKE_STMT *sfstmt) {
    return &sfstmt->error;
}

uint64 STDCALL snowflake_num_rows(SNOWFLAKE_STMT *sfstmt) {
    // TODO fix int vs uint stuff
    return sfstmt->total_rowcount;
}

uint64 STDCALL snowflake_num_fields(SNOWFLAKE_STMT *sfstmt) {
    // TODO fix int vs uint stuff
    return sfstmt->total_fieldcount;
}

uint64 STDCALL snowflake_param_count(SNOWFLAKE_STMT *sfstmt) {
    return sfstmt->params->used;
}

const char *STDCALL snowflake_sfqid(SNOWFLAKE_STMT *sfstmt) {
    // TODO check if sfstmt is NULL for ALL statement functions
    return sfstmt->sfqid;
}

const char *STDCALL snowflake_sqlstate(SNOWFLAKE_STMT *sfstmt) {
    // TODO check if sfstmt is NULL for ALL statement functions
    return sfstmt->sqlstate;
}
