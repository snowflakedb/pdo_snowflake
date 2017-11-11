/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "libsnowflakeclient/include/snowflake_client.h"
#include "connection.h"
#include "snowflake_memory.h"
#include "log.h"
#include "results.h"

#define curl_easier_escape(curl, string) curl_easy_escape(curl, string, 0)

const char CONTENT_TYPE_APPLICATION_JSON[] = "Content-Type: application/json";
const char ACCEPT_TYPE_APPLICATION_SNOWFLAKE[] = "accept: application/snowflake";
const char C_API_USER_AGENT[] = "User-Agent: c_api/0.1";
const char TOKEN_HEADER_FORMAT[] = "Authorization: Snowflake Token=\"%s\"";
const char SESSION_URL[] = "/session/v1/login-request?";
const char QUERY_URL[] = "/queries/v1/query-request?";
static char *LOG_PATH = NULL;
static FILE *LOG_FP = NULL;

/*
 * Convenience method to find string size, create buffer, copy over, and return.
 */
void alloc_buffer_and_copy(char **var, const char *str) {
    size_t str_size;
    sf_free(*var);
    str_size = strlen(str) + 1; // For null terminator
    *var = (char *) sf_calloc(1, str_size);
    strncpy(*var, str, str_size);
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
    int res;
    struct stat st = {0};
    char *directory;
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
    if (sf_log_path != NULL) {
        log_path_size += strlen(sf_log_path);
        log_path_size += 16; // Size of static format characters
        LOG_PATH = (char *) sf_calloc(1, log_path_size);
        snprintf(LOG_PATH, log_path_size, "%s/.capi/logs/%s.txt", sf_log_path, (char *)time_str);
    } else {
        log_path_size += 9; // Size of static format characters
        LOG_PATH = (char *) sf_calloc(1, log_path_size);
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
    sf_free(LOG_PATH);
    if (LOG_FP) {
        fclose(LOG_FP);
    }
}

SNOWFLAKE_STATUS STDCALL snowflake_global_init() {
    SNOWFLAKE_STATUS ret = SF_STATUS_ERROR;
    CURLcode curl_ret;
    // TODO Add log init error handling
    if (!log_init()) {
        fprintf(stderr, "Error during log initialization");
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
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE *STDCALL snowflake_init() {
    // TODO: track memory usage
    SNOWFLAKE *sf = (SNOWFLAKE *) sf_calloc(1, sizeof(SNOWFLAKE));

    // Initialize object with default values
    alloc_buffer_and_copy(&sf->host, "127.0.0.1");
    alloc_buffer_and_copy(&sf->port, "8080");
    sf->user = NULL;
    sf->password = NULL;
    sf->database = NULL;
    sf->account = NULL;
    sf->role = NULL;
    sf->warehouse = NULL;
    sf->schema = NULL;
    alloc_buffer_and_copy(&sf->protocol, "http");
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
        sf_free(sf->host);
        sf_free(sf->port);
        sf_free(sf->user);
        sf_free(sf->password);
        sf_free(sf->database);
        sf_free(sf->account);
        sf_free(sf->role);
        sf_free(sf->warehouse);
        sf_free(sf->schema);
        sf_free(sf->protocol);
        sf_free(sf->passcode);
        sf_free(sf->master_token);
        sf_free(sf->token);
    }
    sf_free(sf);
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
    RAW_JSON_BUFFER *raw_json;
    struct data config;
    config.trace_ascii = 1;
    SNOWFLAKE_STATUS ret = SF_STATUS_ERROR;

    raw_json = (RAW_JSON_BUFFER *) sf_calloc(1, sizeof(RAW_JSON_BUFFER));
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
        }

        /* we are done... */
        ret = SF_STATUS_SUCCESS;
    }

cleanup:
    curl_slist_free_all(header);
    curl_easy_cleanup(curl);
    cJSON_Delete(body);
    cJSON_Delete(resp);
    sf_free(s_body);
    sf_free(s_resp);
    sf_free(raw_json->buffer);
    sf_free(raw_json);
    sf_free(encoded_url);

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
    SNOWFLAKE_STMT *sfstmt = (SNOWFLAKE_STMT *) sf_calloc(1, sizeof(SNOWFLAKE_STMT));
    sfstmt->connection = sf;
    sfstmt->sfqid = NULL;
    sfstmt->sequence_counter = ++sf->sequence_counter;
    uuid4_generate(sfstmt->request_id);
    //sfstmt->error = NULL;
    sfstmt->prepared_command = NULL;
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
    if (sfstmt) {
        sf_free(sfstmt->sfqid);
        if (sfstmt->raw_results) {
            cJSON_Delete(sfstmt->raw_results);
        }
        sf_free(sfstmt->prepared_command);
        array_list_deallocate(sfstmt->params);
        array_list_deallocate(sfstmt->results);
        //TODO deallocate description
    }
    sf_free(sfstmt);
}

SNOWFLAKE_STATUS STDCALL snowflake_bind_param(
    SNOWFLAKE_STMT *sfstmt, SNOWFLAKE_BIND_INPUT *sfbind)
{
    if (sfstmt->params == NULL) {
        sfstmt->params = array_list_create();
    }
    array_list_set(sfstmt->params, sfbind, sfbind->idx);
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_bind_result(
    SNOWFLAKE_STMT *sfstmt, SNOWFLAKE_BIND_OUTPUT *sfbind)
{
    if (sfstmt->results == NULL) {
        sfstmt->results = array_list_create();
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

SNOWFLAKE_STATUS STDCALL snowflake_prepare(
        SNOWFLAKE_STMT *sfstmt, const char *command) {
    size_t prepared_cmd_size = 1; // Don't forget about null terminator
    // If no input params, just set prepared_command to command and return
    if (sfstmt->params == NULL) {
        prepared_cmd_size += strlen(command);
        sfstmt->prepared_command = (char *) sf_calloc(1, prepared_cmd_size);
        strncpy(sfstmt->prepared_command, command, prepared_cmd_size);
        goto cleanup;
    }

    // TODO Actually process params and prepare statement

cleanup:
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_execute(SNOWFLAKE_STMT *sfstmt) {
    CURL *curl;
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
    if (sfstmt->connection->token == NULL || sfstmt->connection->master_token == NULL) {
        if (snowflake_connect(sfstmt->connection) != SF_STATUS_SUCCESS) {
            goto cleanup;
        }
    }
    // -2 since there is a placeholder in the token format; +1 for null terminator
    size_t header_token_size = strlen(TOKEN_HEADER_FORMAT) - 2 + strlen(sfstmt->connection->token) + 1;
    char *encoded_url = NULL;
    URL_KEY_VALUE url_params[] = {
            {"requestId=", sfstmt->request_id, NULL, NULL, 0, 0}
    };
    RAW_JSON_BUFFER *raw_json;
    struct data config;
    config.trace_ascii = 1;
    raw_json = (RAW_JSON_BUFFER *) sf_calloc(1, sizeof(RAW_JSON_BUFFER));
    raw_json->size = 0;
    raw_json->buffer = NULL;

    curl = curl_easy_init();

    if (curl) {
        // Create header
        header_token = (char *) sf_calloc(1, header_token_size);
        snprintf(header_token, header_token_size, TOKEN_HEADER_FORMAT, sfstmt->connection->token);
        header = create_header_token(header_token);
        log_info("Created header with token");

        // Create Body
        body = create_query_json_body(sfstmt->prepared_command, sfstmt->sequence_counter);
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
                    sfstmt->desc = set_description(rowtype);
                    sfstmt->total_fieldcount = cJSON_GetArraySize(rowtype);
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
    sf_free(s_body);
    sf_free(s_resp);
    curl_easy_cleanup(curl);
    sf_free(raw_json->buffer);
    sf_free(raw_json);
    sf_free(header_token);
    sf_free(encoded_url);

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
