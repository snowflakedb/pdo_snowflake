/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "../include/snowflake_client.h"
#include "../deps/curl-7.54.1/include/curl/curl.h"
#include "cJSON.h"

#define curl_easier_escape(curl, string) curl_easy_escape(curl, string, 0)

int8 SF_BOOLEAN_TRUE = 1;
int8 SF_BOOLEAN_FALSE = 0;
const char EMPTY_STRING[] = "";
const char CONTENT_TYPE_APPLICATION_JSON[] = "Content-Type: application/json";
const char ACCEPT_TYPE_APPLICATION_SNOWFLAKE[] = "accept: application/snowflake";
const char C_API_USER_AGENT[] = "User-Agent: c_api/0.1";

cJSON *STDCALL create_json_body(SNOWFLAKE *sf,
                                const char *application,
                                const char *int_app_name,
                                const char *int_app_version) {
    cJSON *body;
    cJSON *data;
    cJSON *client_env;

    //Create Client Environment JSON blob
    client_env = cJSON_CreateObject();
    cJSON_AddStringToObject(client_env, "APPLICATION", application);
    cJSON_AddStringToObject(client_env, "OS_VERSION", "Linux");

    //Create Request Data JSON blob
    data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "CLIENT_APP_ID", int_app_name);
    cJSON_AddStringToObject(data, "CLIENT_APP_VERSION", int_app_version);
    cJSON_AddStringToObject(data, "SVN_REVISION", "12345"); //TODO Add SVN Revision Here
    cJSON_AddStringToObject(data, "ACCOUNT_NAME", sf->account);
    cJSON_AddStringToObject(data, "LOGIN_NAME", sf->user);
    // Add password if one exists
    if (sf->password && *(sf->password)) {
        cJSON_AddStringToObject(data, "PASSWORD", sf->password);
    }
    cJSON_AddItemToObject(data, "CLIENT_ENVIRONMENT", client_env);

    //Create body
    body = cJSON_CreateObject();
    cJSON_AddItemToObject(body, "data", data);


    return body;
}

/*
 * libcurl write function callback to write response to a buffer
 */
int json_resp_cb(char *data, size_t size, size_t nmemb, cJSON **json) {
    // If there is already an allocated JSON object, free it before creating a new one.
    printf("Data input size: %zu\n", strlen(data));
    if (*json) {
        printf("Deleting old JSON blob\n");
        cJSON_Delete(*json);
        *json = NULL;
    }
    *json = cJSON_Parse(data);
    printf("Parsed JSON\n");
    return (int) (size * nmemb);
}

/*
 * Convenience method to find string size, create buffer, copy over, and return.
 */
char *alloc_buffer_and_copy(const char *str) {
    size_t str_size;
    char *buffer;
    str_size = strlen(str);
    buffer = (char *) calloc(1, str_size);
    strncpy(buffer, str, str_size);
    return buffer;
}

SNOWFLAKE_STATUS STDCALL snowflake_global_init() {
    SNOWFLAKE_STATUS ret;
    CURLcode curl_ret;
    curl_ret = curl_global_init(CURL_GLOBAL_DEFAULT);
    if(curl_ret != CURLE_OK) {
        fprintf(stderr, "curl_global_init() failed: %s\n", curl_easy_strerror(curl_ret));
        ret = SF_STATUS_ERROR;
        goto cleanup;
    }

    ret = SF_STATUS_SUCCESS;

cleanup:
    return ret;
}

SNOWFLAKE_STATUS STDCALL snowflake_global_term() {
    curl_global_cleanup();
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE *STDCALL snowflake_init() {
    // TODO: track memory usage
    SNOWFLAKE *sf = (SNOWFLAKE *) calloc(1, sizeof(SNOWFLAKE));

    // Initialize object with default values
    sf->host = alloc_buffer_and_copy("127.0.0.1");
    sf->port = alloc_buffer_and_copy("8080");
    sf->user = NULL;
    sf->password = NULL;
    sf->database = NULL;
    sf->account = NULL;
    sf->role = NULL;
    sf->warehouse = NULL;
    sf->schema = NULL;
    sf->protocol = alloc_buffer_and_copy("http");
    sf->passcode = NULL;
    sf->passcode_in_password = SF_BOOLEAN_FALSE;
    sf->insecure_mode = SF_BOOLEAN_FALSE;
    sf->autocommit = SF_BOOLEAN_FALSE;
    sf->token = NULL;
    sf->master_token = NULL;
    sf->login_timeout = 120;
    sf->network_timeout = 0;

    return sf;
}

void STDCALL snowflake_term(SNOWFLAKE *sf) {
    // TODO: track memory usage
    if (sf->host) {
        free(sf->host);
    }
    if (sf->port) {
        free(sf->port);
    }
    if (sf->user) {
        free(sf->user);
    }
    if (sf->password) {
        free(sf->password);
    }
    if (sf->database) {
        free(sf->database);
    }
    if (sf->account) {
        free(sf->account);
    }
    if (sf->role) {
        free(sf->role);
    }
    if (sf->warehouse) {
        free(sf->warehouse);
    }
    if (sf->schema) {
        free(sf->schema);
    }
    if (sf->protocol) {
        free(sf->protocol);
    }
    if (sf->passcode) {
        free(sf->passcode);
    }
    if (sf->master_token) {
        free(sf->master_token);
    }
    if (sf->token) {
        free(sf->token);
    }
    free(sf);
}

SNOWFLAKE_STATUS STDCALL snowflake_connect(SNOWFLAKE *sf) {
    // TODO: connect to Snowflake
    CURL *curl;
    CURLcode res;
    // Use curl header list
    struct curl_slist *header = NULL;
    FILE *output = NULL;
    cJSON *body = NULL;
    cJSON **resp;
    cJSON *data = NULL;
    cJSON *blob = NULL;
    size_t blob_size;
    char *master_token = NULL;
    char *token = NULL;
    char *s_header = NULL;
    char *s_body = NULL;
    char *s_resp = NULL;
    // Base URL
    const char *url;
    // Encoded URL to use with libcurl
    char *encoded_url;
    char *request_id_value = NULL;
    char *database_value = NULL;
    char *schema_value = NULL;
    char *warehouse_value = NULL;
    char *role_value = NULL;
    const char *request_id_key = "request_id=";
    const char *database_key = NULL;
    const char *schema_key = NULL;
    const char *warehouse_key = NULL;
    const char *role_key = NULL;
    // Used to determine allocation size for encoded url string
    size_t encoded_url_size = 0;
    int bytes_written;
    SNOWFLAKE_STATUS ret = SF_STATUS_ERROR;
    resp = calloc(1, sizeof(cJSON **));
    *resp = NULL;

    curl = curl_easy_init();
    if (curl) {

        // Create header
        header = curl_slist_append(header, CONTENT_TYPE_APPLICATION_JSON);
        header = curl_slist_append(header, ACCEPT_TYPE_APPLICATION_SNOWFLAKE);
        header = curl_slist_append(header, C_API_USER_AGENT);
        printf("Created header\n");

        //Create body
        body = create_json_body(sf, "C API", "C API", "0.1");
        printf("Created body\n");
        url = "/session/v1/login-request?";
        request_id_value = curl_easier_escape(curl, "1");
        if (sf->database && *(sf->database)) {
            database_key = "&databaseName=";
            database_value = curl_easier_escape(curl, sf->database);
        } else {
            database_key = "";
            database_value = curl_easier_escape(curl, "");
        }
        if (sf->schema && *(sf->schema)) {
            schema_key = "&schemaName=";
            schema_value = curl_easier_escape(curl, sf->schema);
        } else {
            schema_key = "";
            schema_value = curl_easier_escape(curl, "");
        }
        if (sf->warehouse && *(sf->warehouse)) {
            warehouse_key = "&warehouse=";
            warehouse_value = curl_easier_escape(curl, sf->warehouse);
        } else {
            warehouse_key = "";
            warehouse_value = curl_easier_escape(curl, "");
        }
        if (sf->role && *(sf->role)) {
            role_key = "&roleName=";
            role_value = curl_easier_escape(curl, sf->role);
        } else {
            role_key = "";
            role_value = curl_easier_escape(curl, "");
        }
        encoded_url_size = strlen(sf->protocol) + strlen(sf->host) + strlen(sf->port) + 4 +
                strlen(url) + strlen(request_id_key) + strlen(request_id_value) +
                strlen(database_key) + strlen(database_value) + strlen(schema_key) + strlen(schema_key) +
                strlen(warehouse_key) + strlen(warehouse_value) + strlen(role_key) + strlen(role_value);
        encoded_url = (char *) calloc(1, encoded_url_size);
        bytes_written = snprintf(encoded_url, encoded_url_size, "%s://%s:%s%s%s%s%s%s%s%s%s%s%s%s",
                 sf->protocol, sf->host, sf->port, url, request_id_key, request_id_value, database_key, database_value,
                 schema_key, schema_value, warehouse_key, warehouse_value, role_key, role_value);

        if (bytes_written < 0 || bytes_written >= encoded_url_size) {
            printf("Encoded url was not properly constructed. Expected size: %zu     Actual Size: %i\n",
                   encoded_url_size, bytes_written);
            goto cleanup;
        }

        //TODO check that string was fully copied

        s_body = cJSON_Print(body);
        //s_header = cJSON_Print(header);
        printf("Here is constructed body:\n%s\n", s_body);
        //printf("\nHere is constructed header:\n%s\n", s_header);
        printf("\nHere is constructed url: %s\n", encoded_url);
        printf("Encoded URL sizes. Expected size: %zu     Actual Size: %i\n", encoded_url_size, bytes_written);

        // Open up file to store curl call info
        output = fopen("/home/kwagner/curl_info.txt", "w+");
        // Setup curl call
        res = curl_easy_setopt(curl, CURLOPT_URL, encoded_url);
        res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, s_body);
        res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp);
        res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &json_resp_cb);
        printf("Running curl call\n");
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            goto cleanup;
        }
        if (*resp) {
            s_resp = cJSON_Print(*resp);
            printf("Here is JSON response:\n%s\n", s_resp);
            data = cJSON_GetObjectItem(*resp, "data");
            // Get token
            blob = cJSON_GetObjectItem(data, "token");
            if (cJSON_IsString(blob)) {
                blob_size = strlen(blob->valuestring);
                if (sf->token) {
                    free(sf->token);
                    sf->token = NULL;
                }
                sf->token = calloc(1, blob_size);
                strncpy(sf->token, blob->valuestring, blob_size);
                printf("Token: %s\n", sf->token);
            } else {
                printf("Token field does not contain a string\n");
            }
            // Get master token
            blob = cJSON_GetObjectItem(data, "masterToken");
            if (cJSON_IsString(blob)) {
                blob_size = strlen(blob->valuestring);
                if (sf->master_token) {
                    free(sf->master_token);
                    sf->master_token = NULL;
                }
                sf->master_token = calloc(1, blob_size);
                strncpy(sf->master_token, blob->valuestring, blob_size);
                printf("Master token: %s\n", sf->master_token);
            } else {
                printf("Master token field does not contain a string\n");
            }
        } else {
            printf("No response\n");
        }

        /* we are done... */
        ret = SF_STATUS_SUCCESS;
    }

cleanup:
    if (header) {
        //cJSON_Delete(header);
        curl_slist_free_all(header);
    }
    if (body) {
        cJSON_Delete(body);
    }
    if (*resp) {
        cJSON_Delete(*resp);
        *resp = NULL;
    }
    if (resp) {
        free(resp);
    }
    if (curl) {
        curl_easy_cleanup(curl);
    }
    if (s_body) {
        cJSON_free(s_body);
    }
    if (s_header) {
        cJSON_free(s_header);
    }
    if (s_resp) {
        cJSON_free(s_resp);
    }
    if (database_value) {
        free(database_value);
    }
    if (schema_value) {
        free(schema_value);
    }
    if (warehouse_value) {
        free(warehouse_value);
    }
    if (role_value) {
        free(role_value);
    }
    if (output) {
        fclose(output);
    }

    return ret;
}

SNOWFLAKE_STATUS STDCALL snowflake_close(SNOWFLAKE *sf) {
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_set_attr(
        SNOWFLAKE *sf, SNOWFLAKE_ATTRIBUTE type, const void *value) {
    if (type == SF_CON_ACCOUNT) {
        if (sf->account) {
            free(sf->account);
        }
        sf->account = alloc_buffer_and_copy(value);
    } else if (type == SF_CON_USER) {
        if (sf->user) {
            free(sf->user);
        }
        sf->user = alloc_buffer_and_copy(value);
    } else if (type == SF_CON_PASSWORD) {
        if (sf->password) {
            free(sf->password);
        }
        sf->password = alloc_buffer_and_copy(value);
    } else if (type == SF_CON_DATABASE) {
        if (sf->database) {
            free(sf->database);
        }
        sf->database = alloc_buffer_and_copy(value);
    } else if (type == SF_CON_SCHEMA) {
        if (sf->schema) {
            free(sf->schema);
        }
        sf->schema = alloc_buffer_and_copy(value);
    } else if (type == SF_CON_WAREHOUSE) {
        if (sf->warehouse) {
            free(sf->warehouse);
        }
        sf->warehouse = alloc_buffer_and_copy(value);
    } else if (type == SF_CON_ROLE) {
        if (sf->role) {
            free(sf->role);
        }
        sf->role = alloc_buffer_and_copy(value);
    } else if (type == SF_CON_HOST) {
        if (sf->host) {
            free(sf->host);
        }
        sf->host = alloc_buffer_and_copy(value);
    } else if (type == SF_CON_PORT) {
        if (sf->port) {
            free(sf->port);
        }
        sf->port = alloc_buffer_and_copy(value);
    } else if (type == SF_CON_PROTOCOL) {
        if (sf->protocol) {
            free(sf->protocol);
        }
        sf->protocol = alloc_buffer_and_copy(value);
    } else if (type == SF_CON_PASSCODE) {
        if (sf->passcode) {
            free(sf->passcode);
        }
        sf->passcode = alloc_buffer_and_copy(value);
    } else if (type == SF_CON_PASSCODE_IN_PASSWORD) {
        sf->passcode_in_password = *((boolean *) value);
    } else if (type == SF_CON_APPLICATION) {
        //TODO Implement this
    } else if (type == SF_CON_AUTHENTICATOR) {
        //TODO Implement this
    } else if (type == SF_CON_INSECURE_MODE) {
        sf->insecure_mode = *((boolean *) value);
    } else if (type == SF_SESSION_PARAMETER) {
        //TODO Implement this
    } else if (type == SF_CON_LOGIN_TIMEOUT) {
        sf->login_timeout = *((int64 *) value);
    } else if (type == SF_CON_NETWORK_TIMEOUT) {
        sf->network_timeout = *((int64 *) value);
    } else if (type == SF_CON_AUTOCOMMIT) {
        sf->autocommit = *((boolean *) value);
    } else {
        return SF_STATUS_ERROR;
    }
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STMT *STDCALL snowflake_stmt(SNOWFLAKE *sf) {
    // TODO: track memory usage
    SNOWFLAKE_STMT *ret = (SNOWFLAKE_STMT *) calloc(1, sizeof(SNOWFLAKE_STMT));
    ret->connection = sf;
    return ret;
}

void STDCALL snowflake_stmt_close(SNOWFLAKE_STMT *sfstmt) {
    // TODO: track memory usage
    free(sfstmt);
}

SNOWFLAKE_STATUS STDCALL snowflake_bind_param(
    SNOWFLAKE_STMT *sfstmt, SNOWFLAKE_BIND_INPUT *sfbind)
{
  return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_bind_result(
    SNOWFLAKE_STMT *sfstmt, SNOWFLAKE_BIND_OUTPUT *sfbind_array)
{
  return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_query(
        SNOWFLAKE_STMT *sfstmt, const char *command) {
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_fetch(SNOWFLAKE_STMT *sfstmt) {
    return SF_STATUS_SUCCESS;
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

int64 STDCALL snowflake_affected_rows(SNOWFLAKE_STMT *sfstmt) {
    return 0;
}

SNOWFLAKE_STATUS STDCALL snowflake_prepare(
        SNOWFLAKE_STMT *sfstmt, const char *command) {
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_execute(SNOWFLAKE_STMT *sfstmt) {
    return SF_STATUS_SUCCESS;
}

SNOWFLAKE_ERROR *STDCALL snowflake_error(SNOWFLAKE_STMT *sfstmt) {
    return &sfstmt->error;
}

uint64 STDCALL snowflake_num_rows(SNOWFLAKE_STMT *sfstmt) {
    return (uint64) 1;
}

const char *STDCALL snowflake_sfqid(SNOWFLAKE_STMT *sfstmt) {
    // TODO check if sfstmt is NULL
    return sfstmt->sfqid;
}
