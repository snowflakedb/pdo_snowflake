/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <string.h>
#include "connection.h"
#include <log.h>
#include "snowflake_memory.h"
#include "snowflake_client_int.h"
#include "constants.h"
#include "error.h"

#define curl_easier_escape(curl, string) curl_easy_escape(curl, string, 0)
#define QUERYCODE_LEN 7

static
void dump(const char *text,
          FILE *stream, unsigned char *ptr, size_t size,
          char nohex)
{
    size_t i;
    size_t c;

    unsigned int width = 0x10;

    if(nohex)
        /* without the hex output, we can fit more on screen */
        width = 0x40;

    fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n",
            text, (long)size, (long)size);

    for(i = 0; i<size; i += width) {

        fprintf(stream, "%4.4lx: ", (long)i);

        if(!nohex) {
            /* hex not disabled, show it */
            for(c = 0; c < width; c++)
                if(i + c < size)
                    fprintf(stream, "%02x ", ptr[i + c]);
                else
                    fputs("   ", stream);
        }

        for(c = 0; (c < width) && (i + c < size); c++) {
            /* check for 0D0A; if found, skip past and start a new line of output */
            if(nohex && (i + c + 1 < size) && ptr[i + c] == 0x0D &&
               ptr[i + c + 1] == 0x0A) {
                i += (c + 2 - width);
                break;
            }
            fprintf(stream, "%c",
                    (ptr[i + c] >= 0x20) && (ptr[i + c]<0x80)?ptr[i + c]:'.');
            /* check again for 0D0A, to avoid an extra \n if it's at width */
            if(nohex && (i + c + 2 < size) && ptr[i + c + 1] == 0x0D &&
               ptr[i + c + 2] == 0x0A) {
                i += (c + 3 - width);
                break;
            }
        }
        fputc('\n', stream); /* newline */
    }
    fflush(stream);
}

static
int my_trace(CURL *handle, curl_infotype type,
             char *data, size_t size,
             void *userp)
{
    struct data *config = (struct data *)userp;
    const char *text;
    (void)handle; /* prevent compiler warning */

    switch(type) {
        case CURLINFO_TEXT:
            fprintf(stderr, "== Info: %s", data);
            /* FALLTHROUGH */
        default: /* in case a new one is introduced to shock us */
            return 0;

        case CURLINFO_HEADER_OUT:
            text = "=> Send header";
            break;
        case CURLINFO_DATA_OUT:
            text = "=> Send data";
            break;
        case CURLINFO_SSL_DATA_OUT:
            text = "=> Send SSL data";
            break;
        case CURLINFO_HEADER_IN:
            text = "<= Recv header";
            break;
        case CURLINFO_DATA_IN:
            text = "<= Recv data";
            break;
        case CURLINFO_SSL_DATA_IN:
            text = "<= Recv SSL data";
            break;
    }

    dump(text, stderr, (unsigned char *)data, size, config->trace_ascii);
    return 0;
}

static uint32 uimin(uint32 a, uint32 b) {
    return (a < b) ? a : b;
}

static uint32 uimax(uint32 a, uint32 b) {
    return (a > b) ? a : b;
}


cJSON *STDCALL create_auth_json_body(SF_CONNECT *sf,
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

cJSON *STDCALL create_query_json_body(const char *sql_text, int64 sequence_id) {
    cJSON *body;
    // Create body
    body = cJSON_CreateObject();
    cJSON_AddStringToObject(body, "sqlText", sql_text);
    cJSON_AddBoolToObject(body, "asyncExec", SF_BOOLEAN_FALSE);
    cJSON_AddNumberToObject(body, "sequenceId", sequence_id);

    return body;
}

cJSON *STDCALL create_renew_session_json_body(const char *old_token) {
    cJSON *body;
    // Create body
    body = cJSON_CreateObject();
    cJSON_AddStringToObject(body, "oldSessionToken", old_token);
    cJSON_AddStringToObject(body, "requestType", REQUEST_TYPE_RENEW);

    return body;
}

struct curl_slist * STDCALL create_header_no_token() {
    struct curl_slist * header = NULL;
    header = curl_slist_append(header, HEADER_CONTENT_TYPE_APPLICATION_JSON);
    header = curl_slist_append(header, HEADER_ACCEPT_TYPE_APPLICATION_SNOWFLAKE);
    header = curl_slist_append(header, HEADER_C_API_USER_AGENT);
    return header;
}

struct curl_slist * STDCALL create_header_token(const char *header_token) {
    struct curl_slist * header = NULL;
    header = curl_slist_append(header, header_token);
    header = curl_slist_append(header, HEADER_CONTENT_TYPE_APPLICATION_JSON);
    header = curl_slist_append(header, HEADER_ACCEPT_TYPE_APPLICATION_SNOWFLAKE);
    header = curl_slist_append(header, HEADER_C_API_USER_AGENT);
    return header;
}

sf_bool STDCALL curl_post_call(SF_CONNECT *sf,
                               CURL *curl,
                               char *url,
                               struct curl_slist *header,
                               char *body,
                               cJSON **json,
                               SF_ERROR *error) {
    const char *error_msg;
    SF_JSON_ERROR json_error;
    char query_code[QUERYCODE_LEN];
    char *result_url = NULL;
    cJSON *data = NULL;
    struct curl_slist *new_header = NULL;
    size_t header_token_size;
    char *header_token = NULL;
    sf_bool ret = SF_BOOLEAN_FALSE;
    sf_bool stop = SF_BOOLEAN_FALSE;

    // Set to 0
    memset(query_code, 0, QUERYCODE_LEN);

    do {
        if(!http_perform(sf, curl, POST_REQUEST_TYPE, url, header, body, json, error) || !*json) {
            // Error is set in the perform function
            break;
        }
        if ((json_error = json_copy_string_no_alloc(query_code, *json, "code", QUERYCODE_LEN)) != SF_JSON_ERROR_NONE &&
                json_error != SF_JSON_ERROR_ITEM_NULL) {
            JSON_ERROR_MSG(json_error, error_msg, "Query code");
            SET_SNOWFLAKE_ERROR(error, SF_ERROR_BAD_JSON, error_msg, SF_SQLSTATE_UNABLE_TO_CONNECT);
            break;
        }

        // No query code means things went well, just break and return
        if (!query_code) {
            ret = SF_BOOLEAN_TRUE;
            break;
        }

        if (strcmp(query_code, SESSION_EXPIRE_CODE) == 0) {
            if (!renew_session(curl, sf, error)) {
                // Error is set in renew session function
                break;
            } else {
                // TODO refactor creating the header token string
                // Create new header since we have a new token
                header_token_size = strlen(HEADER_SNOWFLAKE_TOKEN_FORMAT) - 2 + strlen(sf->token) + 1;
                header_token = (char *) SF_CALLOC(1, header_token_size);
                if (!header_token) {
                    SET_SNOWFLAKE_ERROR(error, SF_ERROR_OUT_OF_MEMORY,
                                        "Ran out of memory trying to create header token", SF_SQLSTATE_UNABLE_TO_CONNECT);
                    break;
                }
                snprintf(header_token, header_token_size, HEADER_SNOWFLAKE_TOKEN_FORMAT, sf->token);
                new_header = create_header_token(header_token);
                if (!curl_post_call(sf, curl, url, new_header, body, json, error)) {
                    // Error is set in curl call
                    break;
                }
            }
        }

        while (query_code && (strcmp(query_code, QUERY_IN_PROGRESS_CODE) == 0 || strcmp(query_code, QUERY_IN_PROGRESS_ASYNC_CODE) == 0)) {
            // Remove old result URL and query code if this isn't our first rodeo
            SF_FREE(result_url);
            memset(query_code, 0, QUERYCODE_LEN);
            data = cJSON_GetObjectItem(*json, "data");
            if (json_copy_string(&result_url, data, "getResultUrl") != SF_JSON_ERROR_NONE) {
                stop = SF_BOOLEAN_TRUE;
                JSON_ERROR_MSG(json_error, error_msg, "Result URL");
                SET_SNOWFLAKE_ERROR(error, SF_ERROR_BAD_JSON, error_msg, SF_SQLSTATE_UNABLE_TO_CONNECT);
                break;
            }

            log_debug("ping pong starting...");
            if (!request(sf, json, result_url, NULL, 0, NULL, header, GET_REQUEST_TYPE, error)) {
                // Error came from request up, just break
                stop = SF_BOOLEAN_TRUE;
                break;
            }

            if ((json_error = json_copy_string_no_alloc(query_code, *json, "code", QUERYCODE_LEN)) != SF_JSON_ERROR_NONE &&
                json_error != SF_JSON_ERROR_ITEM_NULL) {
                stop = SF_BOOLEAN_TRUE;
                JSON_ERROR_MSG(json_error, error_msg, "Query code");
                SET_SNOWFLAKE_ERROR(error, SF_ERROR_BAD_JSON, error_msg, SF_SQLSTATE_UNABLE_TO_CONNECT);
                break;
            }
        }

        if (stop) {
            break;
        }

        ret = SF_BOOLEAN_TRUE;
    } while (0); // Dummy loop to break out of

    SF_FREE(result_url);
    SF_FREE(header_token);
    curl_slist_free_all(new_header);

    return ret;
}

sf_bool STDCALL curl_get_call(SF_CONNECT *sf,
                              CURL *curl,
                              char *url,
                              struct curl_slist *header,
                              cJSON **json,
                              SF_ERROR *error) {
    SF_JSON_ERROR json_error;
    const char *error_msg;
    char query_code[QUERYCODE_LEN];
    char *result_url = NULL;
    struct curl_slist *new_header = NULL;
    size_t header_token_size;
    char *header_token = NULL;
    sf_bool ret = SF_BOOLEAN_FALSE;

    // Set to 0
    memset(query_code, 0, QUERYCODE_LEN);

    do {
        if(!http_perform(sf, curl, GET_REQUEST_TYPE, url, header, NULL, json, error) || !*json) {
            // Error is set in the perform function
            break;
        }
        if ((json_error = json_copy_string_no_alloc(query_code, *json, "code", QUERYCODE_LEN)) != SF_JSON_ERROR_NONE &&
            json_error != SF_JSON_ERROR_ITEM_NULL) {
            JSON_ERROR_MSG(json_error, error_msg, "Query code");
            SET_SNOWFLAKE_ERROR(error, SF_ERROR_BAD_JSON, error_msg, SF_SQLSTATE_UNABLE_TO_CONNECT);
            break;
        }

        // No query code means things went well, just break and return
        if (!query_code) {
            ret = SF_BOOLEAN_TRUE;
            break;
        }

        if (strcmp(query_code, SESSION_EXPIRE_CODE) == 0) {
            if (!renew_session(curl, sf, error)) {
                // Error is set in renew session function
                break;
            } else {
                // TODO refactor creating the header token string
                // Create new header since we have a new token
                header_token_size = strlen(HEADER_SNOWFLAKE_TOKEN_FORMAT) - 2 + strlen(sf->token) + 1;
                header_token = (char *) SF_CALLOC(1, header_token_size);
                if (!header_token) {
                    SET_SNOWFLAKE_ERROR(error, SF_ERROR_OUT_OF_MEMORY,
                                        "Ran out of memory trying to create header token", SF_SQLSTATE_UNABLE_TO_CONNECT);
                    break;
                }
                snprintf(header_token, header_token_size, HEADER_SNOWFLAKE_TOKEN_FORMAT, sf->token);
                new_header = create_header_token(header_token);
                if (!curl_get_call(sf, curl, url, new_header, json, error)) {
                    // Error is set in curl call
                    break;
                }
            }
        }

        ret = SF_BOOLEAN_TRUE;
    } while (0); // Dummy loop to break out of

    SF_FREE(result_url);
    SF_FREE(header_token);
    curl_slist_free_all(new_header);

    return ret;
}

void STDCALL decorrelate_jitter_free(DECORRELATE_JITTER_BACKOFF *djb) {
    SF_FREE(djb);
}

DECORRELATE_JITTER_BACKOFF *STDCALL decorrelate_jitter_init(uint32 base, uint32 cap) {
    DECORRELATE_JITTER_BACKOFF *djb = (DECORRELATE_JITTER_BACKOFF *) SF_CALLOC(1, sizeof(DECORRELATE_JITTER_BACKOFF));
    djb->base = base;
    djb->cap = cap;
    return djb;
}

uint32 decorrelate_jitter_next_sleep(DECORRELATE_JITTER_BACKOFF *djb, uint32 sleep) {
    return uimin(djb->cap, uimax(djb->base, (uint32) (rand() % (sleep * 3))));
}

char * encode_url(CURL *curl,
                  const char *protocol,
                  const char *account,
                  const char *host,
                  const char *port,
                  const char *url,
                  URL_KEY_VALUE* vars,
                  int num_args,
                  SF_ERROR *error) {
    int i;
    sf_bool host_empty = is_string_empty(host);
    sf_bool port_empty = is_string_empty(port);
    const char *format;
    char *encoded_url = NULL;
    // Size used for the url format
    size_t base_url_size = 1; //Null terminator
    // Size used to determine buffer size
    size_t encoded_url_size;
    // Set proper format based on variables passed into encode URL.
    // The format includes format specifiers that will be consumed by empty fields
    // (i.e if port is empty, add an extra specifier so that we have 1 call to snprintf, vs. 4 different calls)
    // Format specifier order is protocol, then account, then host, then port, then url.
    // Base size increases reflect the number of static characters in the format string (i.e. ':', '/', '.')
    if (!port_empty && !host_empty) {
        format = "%s://%s%s:%s%s";
        base_url_size += 4;
        // Set account to an empty string since host overwrites account
        account = "";
    } else if(port_empty && !host_empty) {
        format = "%s://%s%s%s%s";
        base_url_size += 3;
        port = "";
        // Set account to an empty string since host overwrites account
        account = "";
    } else if(!port_empty && host_empty) {
        format = "%s://%s.%s:%s%s";
        base_url_size += 5;
        host = DEFAULT_SNOWFLAKE_BASE_URL;
    } else {
        format = "%s://%s.%s%s%s";
        base_url_size += 4;
        host = DEFAULT_SNOWFLAKE_BASE_URL;
        port = "";
    }
    base_url_size += strlen(protocol) + strlen(account) + strlen(host) + strlen(port) + strlen(url);

    encoded_url_size = base_url_size;
    // Encode URL parameters and set size info
    for (i = 0; i < num_args; i++) {
        if(vars[i].value && *vars[i].value) {
            vars[i].formatted_key = vars[i].key;
            vars[i].formatted_value = curl_easier_escape(curl, vars[i].value);
        } else {
            vars[i].formatted_key = "";
            vars[i].formatted_value = curl_easier_escape(curl, "");
        }
        vars[i].key_size = strlen(vars[i].formatted_key);
        vars[i].value_size = strlen(vars[i].formatted_value);
        encoded_url_size += vars[i].key_size + vars[i].value_size;
    }

    encoded_url = (char *) SF_CALLOC(1, encoded_url_size);
    if (!encoded_url) {
        SET_SNOWFLAKE_ERROR(error, SF_ERROR_OUT_OF_MEMORY, "Ran out of memory trying to create encoded url", SF_SQLSTATE_UNABLE_TO_CONNECT);
        goto cleanup;
    }
    snprintf(encoded_url, base_url_size, format, protocol, account, host, port, url);

    // Add encoded URL parameters to encoded_url buffer
    for (i = 0; i < num_args; i++) {
        strncat(encoded_url, vars[i].formatted_key, vars[i].key_size);
        strncat(encoded_url, vars[i].formatted_value, vars[i].value_size);
    }

    log_debug("Here is constructed url: %s", encoded_url);

cleanup:
    // Free created memory
    for (i = 0; i < num_args; i++) {
        SF_FREE(vars[i].formatted_value);
    }

    return encoded_url;
}

sf_bool is_string_empty(const char *str) {
    return (str && strcmp(str, "") != 0) ? SF_BOOLEAN_FALSE : SF_BOOLEAN_TRUE;
}

SF_JSON_ERROR STDCALL json_copy_string(char **dest, cJSON *data, const char *item) {
    size_t blob_size;
    cJSON *blob = cJSON_GetObjectItem(data, item);
    if (!blob) {
        return SF_JSON_ERROR_ITEM_MISSING;
    } else if(cJSON_IsNull(blob)) {
        return SF_JSON_ERROR_ITEM_NULL;
    } else if (!cJSON_IsString(blob)) {
        return SF_JSON_ERROR_ITEM_WRONG_TYPE;
    } else {
        blob_size = strlen(blob->valuestring) + 1;
        SF_FREE(*dest);
        *dest = (char *) SF_CALLOC(1, blob_size);
        if (!*dest) {
            return SF_JSON_ERROR_OOM;
        }
        strncpy(*dest, blob->valuestring, blob_size);
        log_debug("Found item and value; %s: %s", item, *dest);
    }

    return SF_JSON_ERROR_NONE;
}

SF_JSON_ERROR STDCALL json_copy_string_no_alloc(char *dest, cJSON *data, const char *item, size_t dest_size) {
    cJSON *blob = cJSON_GetObjectItem(data, item);
    if (!blob) {
        return SF_JSON_ERROR_ITEM_MISSING;
    } else if(cJSON_IsNull(blob)) {
        return SF_JSON_ERROR_ITEM_NULL;
    } else if (!cJSON_IsString(blob)) {
        return SF_JSON_ERROR_ITEM_WRONG_TYPE;
    } else {
        strncpy(dest, blob->valuestring, dest_size);
        // If string is not null terminated, then add the terminator yourself
        if (dest[dest_size - 1] != '\0') {
            dest[dest_size - 1] = '\0';
        }
        log_debug("Found item and value; %s: %s", item, dest);
    }

    return SF_JSON_ERROR_NONE;
}

SF_JSON_ERROR STDCALL json_copy_bool(sf_bool *dest, cJSON *data, const char *item) {
    cJSON *blob = cJSON_GetObjectItem(data, item);
    if (!blob) {
        return SF_JSON_ERROR_ITEM_MISSING;
    } else if(cJSON_IsNull(blob)) {
        return SF_JSON_ERROR_ITEM_NULL;
    } else if (!cJSON_IsBool(blob)) {
        return SF_JSON_ERROR_ITEM_WRONG_TYPE;
    } else {
        *dest = cJSON_IsTrue(blob) ? SF_BOOLEAN_TRUE : SF_BOOLEAN_FALSE;
        log_debug("Found item and value; %s: %i", item, *dest);
    }

    return SF_JSON_ERROR_NONE;
}

SF_JSON_ERROR STDCALL json_copy_int(int64 *dest, cJSON *data, const char *item) {
    cJSON *blob = cJSON_GetObjectItem(data, item);
    if (!blob) {
        return SF_JSON_ERROR_ITEM_MISSING;
    } else if(cJSON_IsNull(blob)) {
        return SF_JSON_ERROR_ITEM_NULL;
    } else if (!cJSON_IsNumber(blob)) {
        return SF_JSON_ERROR_ITEM_WRONG_TYPE;
    } else {
        *dest = (int64) blob->valuedouble;
        log_debug("Found item and value; %s: %i", item, *dest);
    }

    return SF_JSON_ERROR_NONE;
}

SF_JSON_ERROR STDCALL json_detach_array_from_object(cJSON **dest, cJSON *data, const char *item) {
    cJSON *blob = cJSON_DetachItemFromObject(data, item);
    if (!blob) {
        return SF_JSON_ERROR_ITEM_MISSING;
    } else if(cJSON_IsNull(blob)) {
        return SF_JSON_ERROR_ITEM_NULL;
    } else if (!cJSON_IsArray(blob)) {
        return SF_JSON_ERROR_ITEM_WRONG_TYPE;
    } else {
        if (*dest) {
            cJSON_Delete(*dest);
        }
        *dest = blob;
        log_debug("Found array item: %s", item);
    }

    return SF_JSON_ERROR_NONE;
}

SF_JSON_ERROR STDCALL json_detach_array_from_array(cJSON **dest, cJSON *data, int index) {
    cJSON *blob = cJSON_DetachItemFromArray(data, index);
    if (!blob) {
        return SF_JSON_ERROR_ITEM_MISSING;
    } else if(cJSON_IsNull(blob)) {
        return SF_JSON_ERROR_ITEM_NULL;
    } else if (!cJSON_IsArray(blob)) {
        return SF_JSON_ERROR_ITEM_WRONG_TYPE;
    } else {
        if (*dest) {
            cJSON_Delete(*dest);
        }
        *dest = blob;
        log_debug("Found array item at index: %s", index);
    }

    return SF_JSON_ERROR_NONE;
}

/**
 * libcurl write function callback to write response to a buffer
 */
size_t json_resp_cb(char *data, size_t size, size_t nmemb, RAW_JSON_BUFFER *raw_json) {
    size_t data_size = size * nmemb;
    log_debug("Curl response size: %zu\n", data_size);
    raw_json->buffer = (char *) SF_REALLOC(raw_json->buffer, raw_json->size + data_size + 1);
    // Start copying where last null terminator existed
    memcpy(&raw_json->buffer[raw_json->size], data, data_size);
    raw_json->size += data_size;
    // Set null terminator
    raw_json->buffer[raw_json->size] = '\0';
    return data_size;
}

sf_bool STDCALL http_perform(SF_CONNECT *sf,
                             CURL *curl,
                             SF_REQUEST_TYPE request_type,
                             char *url,
                             struct curl_slist *header,
                             char *body,
                             cJSON **json,
                             SF_ERROR *error) {
    CURLcode res;
    sf_bool ret = SF_BOOLEAN_FALSE;
    sf_bool retry = SF_BOOLEAN_FALSE;
    int32 http_code = 0;
    DECORRELATE_JITTER_BACKOFF djb = {
            1,      //base
            16      //cap
    };
    RETRY_CONTEXT retry_ctx = {
            0,      //retry_count
            sf->network_timeout,
            1,      // time to sleep
            &djb    // Decorrelate jitter
    };
    RAW_JSON_BUFFER buffer = {NULL, 0};
    struct data config;
    config.trace_ascii = 1;

    if (curl == NULL) {
        return SF_BOOLEAN_FALSE;
    }

    //TODO set error buffer

    do {
        // Set parameters
        res = curl_easy_setopt(curl, CURLOPT_URL, url);
        if (res != CURLE_OK) {
            log_error("Failed to set URL [%s]", curl_easy_strerror(res));
            break;
        }

        if (DEBUG) {
            curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
            curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &config);

            /* the DEBUGFUNCTION has no effect until we enable VERBOSE */
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        }

        res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        if (res != CURLE_OK) {
            log_error("Failed to set header [%s]", curl_easy_strerror(res));
            break;
        }

        // Post type stuffs
        if (request_type == POST_REQUEST_TYPE) {
            res = curl_easy_setopt(curl, CURLOPT_POST, 1);
            if (res != CURLE_OK) {
                log_error("Failed to set post [%s]", curl_easy_strerror(res));
                break;
            }

            if (body) {
                res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
            } else {
                res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
            }
            if (res != CURLE_OK) {
                log_error("Failed to set body [%s]", curl_easy_strerror(res));
                break;
            }
        }


        res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &json_resp_cb);
        if (res != CURLE_OK) {
            log_error("Failed to set writer [%s]", curl_easy_strerror(res));
            break;
        }

        res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &buffer);
        if (res != CURLE_OK) {
            log_error("Failed to set write data [%s]", curl_easy_strerror(res));
            break;
        }

        if (DISABLE_VERIFY_PEER) {
            res = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            if (res != CURLE_OK) {
                log_error("Failed to disable peer verification [%s]", curl_easy_strerror(res));
                break;
            }
        }

        if (CA_BUNDLE_FILE) {
            res = curl_easy_setopt(curl, CURLOPT_CAINFO, CA_BUNDLE_FILE);
            if (res != CURLE_OK) {
                log_error("Unable to set certificate file [%s]", curl_easy_strerror(res));
                break;
            }
        }

        res = curl_easy_setopt(curl, CURLOPT_SSLVERSION, SSL_VERSION);
        if (res != CURLE_OK) {
            log_error("Unable to set SSL Version [%s]", curl_easy_strerror(res));
            break;
        }

        // Be optimistic
        retry = SF_BOOLEAN_FALSE;

        log_debug("Running curl call");
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK) {
            log_error("curl_easy_perform() failed: %s", curl_easy_strerror(res));
            SET_SNOWFLAKE_ERROR(error, SF_ERROR_CURL, "Failed during easy perform", SF_SQLSTATE_UNABLE_TO_CONNECT);
        } else {
            if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code) != CURLE_OK) {
                log_error("Unable to get http response code [%s]", curl_easy_strerror(res));
                SET_SNOWFLAKE_ERROR(error, SF_ERROR_CURL, "Unable to get http response code", SF_SQLSTATE_UNABLE_TO_CONNECT);
            } else if (http_code != 200) {
                retry = is_retryable_http_code(http_code);
                if (!retry) {
                    SET_SNOWFLAKE_ERROR(error, SF_ERROR_RETRY, "Received unretryable http code", SF_SQLSTATE_UNABLE_TO_CONNECT);
                }
            } else {
                ret = SF_BOOLEAN_TRUE;
            }
        }

        // Reset everything
        reset_curl(curl);
        http_code = 0;

    } while(retry);

    // We were successful so parse JSON from text
    if (ret) {
        cJSON_Delete(*json);
        *json = NULL;
        *json = cJSON_Parse(buffer.buffer);
        if (*json) {
            ret = SF_BOOLEAN_TRUE;
        } else {
            SET_SNOWFLAKE_ERROR(error, SF_ERROR_BAD_JSON, "Unable to parse JSON text response.", SF_SQLSTATE_UNABLE_TO_CONNECT);
            ret = SF_BOOLEAN_FALSE;
        }
    }

    SF_FREE(buffer.buffer);

    return ret;
}

sf_bool STDCALL is_retryable_http_code(int32 code) {
    return ((code >= 500 && code < 600) || code == 400 || code == 403 || code == 408) ? SF_BOOLEAN_TRUE : SF_BOOLEAN_FALSE;
}

sf_bool STDCALL request(SF_CONNECT *sf,
                        cJSON **json,
                        const char *url,
                        URL_KEY_VALUE* url_params,
                        int num_url_params,
                        char *body,
                        struct curl_slist *header,
                        SF_REQUEST_TYPE request_type,
                        SF_ERROR *error) {
    sf_bool ret = SF_BOOLEAN_FALSE;
    CURL *curl = NULL;
    char *encoded_url = NULL;
    struct curl_slist *my_header = NULL;
    char *header_token = NULL;
    size_t header_token_size;

    curl = curl_easy_init();
    if (curl) {
        // Use passed in header if one exists
        if (header) {
            my_header = header;
        } else {
            // Create header
            if (sf->token) {
                header_token_size = strlen(HEADER_SNOWFLAKE_TOKEN_FORMAT) - 2 + strlen(sf->token) + 1;
                header_token = (char *) SF_CALLOC(1, header_token_size);
                if (!header_token) {
                    SET_SNOWFLAKE_ERROR(error, SF_ERROR_OUT_OF_MEMORY,
                                        "Ran out of memory trying to create header token", SF_SQLSTATE_UNABLE_TO_CONNECT);
                    goto cleanup;
                }
                snprintf(header_token, header_token_size, HEADER_SNOWFLAKE_TOKEN_FORMAT, sf->token);
                my_header = create_header_token(header_token);
            } else {
                my_header = create_header_no_token();
            }
            log_debug("Created header");
        }

        encoded_url = encode_url(curl, sf->protocol, sf->account, sf->host, sf->port, url, url_params, num_url_params, error);
        if (encoded_url == NULL) {
            goto cleanup;
        }

        // Execute request and set return value to result
        if (request_type == POST_REQUEST_TYPE) {
            ret = curl_post_call(sf, curl, encoded_url, my_header, body, json, error);
        } else if (request_type == GET_REQUEST_TYPE) {
            ret = curl_get_call(sf, curl, encoded_url, my_header, json, error);
        } else {
            SET_SNOWFLAKE_ERROR(error, SF_ERROR_BAD_REQUEST,
                                "An unknown request type was passed to the request function", SF_SQLSTATE_UNABLE_TO_CONNECT);
            goto cleanup;
        }
    }

cleanup:
    // If we created our own header, then delete it
    if (!header) {
        curl_slist_free_all(my_header);
    }
    curl_easy_cleanup(curl);
    SF_FREE(header_token);
    SF_FREE(encoded_url);

    return ret;
}

sf_bool STDCALL renew_session(CURL *curl, SF_CONNECT *sf, SF_ERROR *error) {
    sf_bool ret = SF_BOOLEAN_FALSE;
    SF_JSON_ERROR json_error;
    const char *error_msg = NULL;
    char request_id[UUID4_LEN];
    struct curl_slist *header = NULL;
    cJSON *body = NULL;
    cJSON *json = NULL;
    char *s_body = NULL;
    char *encoded_url = NULL;
    char *header_token = NULL;
    size_t header_token_size;
    sf_bool success = SF_BOOLEAN_FALSE;
    cJSON *data = NULL;
    cJSON_bool has_token = 0;
    URL_KEY_VALUE url_params[] = {
            {"request_id=", NULL, NULL, NULL, 0, 0},
    };
    if (!curl) {
        return ret;
    }
    if (is_string_empty(sf->master_token)) {
        SET_SNOWFLAKE_ERROR(error, SF_ERROR_BAD_REQUEST, "Missing master token when trying to renew session. "
                "Are you sure your connection was properly setup?", SF_SQLSTATE_UNABLE_TO_CONNECT);
        return ret;
    }
    log_debug("Updating session. Master token: %s", sf->master_token);
    // Create header
    header_token_size = strlen(HEADER_SNOWFLAKE_TOKEN_FORMAT) - 2 + strlen(sf->master_token) + 1;
    header_token = (char *) SF_CALLOC(1, header_token_size);
    if (!header_token) {
        SET_SNOWFLAKE_ERROR(error, SF_ERROR_OUT_OF_MEMORY,
                            "Ran out of memory trying to create header token", SF_SQLSTATE_UNABLE_TO_CONNECT);
        goto cleanup;
    }
    snprintf(header_token, header_token_size, HEADER_SNOWFLAKE_TOKEN_FORMAT, sf->master_token);
    header = create_header_token(header_token);

    // Create body and convert to string
    body = create_renew_session_json_body(sf->token);
    s_body = cJSON_Print(body);

    // Create request id, set in url parameter and encode url
    uuid4_generate(request_id);
    url_params[0].value = request_id;
    encoded_url = encode_url(curl, sf->protocol, sf->account, sf->host, sf->port, RENEW_SESSION_URL, url_params, 1, error);
    if (!encoded_url) {
        goto cleanup;
    }

    // Successful call, non-null json, successful success code, data object and session token must all be present
    // otherwise set an error
    if (!curl_post_call(sf, curl, encoded_url, header, s_body, &json, error) || !json) {
        // Do nothing, let error propogate up from post call
        log_error("Curl call failed during renew session");
        goto cleanup;
    } else if ((json_error = json_copy_bool(&success, json, "success"))) {
        log_error("Error finding success in JSON response for renew session");
        JSON_ERROR_MSG(json_error, error_msg, "Success");
        SET_SNOWFLAKE_ERROR(error, SF_ERROR_BAD_JSON, error_msg, SF_SQLSTATE_UNABLE_TO_CONNECT);
        goto cleanup;
    } else if (!success) {
        log_error("Renew session was unsuccessful");
        SET_SNOWFLAKE_ERROR(error, SF_ERROR_BAD_RESPONSE, "Request returned as being unsuccessful", SF_SQLSTATE_UNABLE_TO_CONNECT);
        goto cleanup;
    } else if (!(data = cJSON_GetObjectItem(json, "data"))) {
        log_error("Missing data field in response");
        SET_SNOWFLAKE_ERROR(error, SF_ERROR_BAD_JSON, "No data object in JSON response", SF_SQLSTATE_UNABLE_TO_CONNECT);
        goto cleanup;
    } else if (!(has_token = cJSON_HasObjectItem(data, "sessionToken"))) {
        log_error("No session token in JSON response");
        SET_SNOWFLAKE_ERROR(error, SF_ERROR_BAD_JSON, "No session token in JSON response", SF_SQLSTATE_UNABLE_TO_CONNECT);
        goto cleanup;
    } else {
        log_debug("Successful renew session");
        if (!set_tokens(sf, data, "sessionToken", "masterToken", error)) {
            goto cleanup;
        }
        log_debug("Finished updating session");
    }

    ret = SF_BOOLEAN_TRUE;

cleanup:
    curl_slist_free_all(header);
    cJSON_Delete(body);
    cJSON_Delete(json);
    SF_FREE(s_body);
    SF_FREE(header_token);
    SF_FREE(encoded_url);

    return ret;
}

void STDCALL reset_curl(CURL *curl) {
    curl_easy_reset(curl);
}

void STDCALL retry_ctx_free(RETRY_CONTEXT *retry_ctx) {
    SF_FREE(retry_ctx);
}

RETRY_CONTEXT *STDCALL retry_ctx_init(uint64 timeout) {
    RETRY_CONTEXT *retry_ctx = (RETRY_CONTEXT *) SF_CALLOC(1, sizeof(RETRY_CONTEXT));
    retry_ctx->retry_timeout = timeout;
    retry_ctx->retry_count = 0;
    retry_ctx->sleep_time = 1;
    retry_ctx->djb = decorrelate_jitter_init(1, 16);
    return retry_ctx;
}

uint32 STDCALL retry_ctx_next_sleep(RETRY_CONTEXT *retry_ctx) {
    retry_ctx->sleep_time = decorrelate_jitter_next_sleep(retry_ctx->djb, retry_ctx->sleep_time);
    return retry_ctx->sleep_time;
}

sf_bool STDCALL set_tokens(SF_CONNECT *sf,
                           cJSON *data,
                           const char *session_token_str,
                           const char *master_token_str,
                           SF_ERROR *error) {
    // Get token
    if (json_copy_string(&sf->token, data, session_token_str)) {
        log_error("No valid token found in response");
        SET_SNOWFLAKE_ERROR(error, SF_ERROR_BAD_JSON, "Cannot find valid session token in response", SF_SQLSTATE_UNABLE_TO_CONNECT);
        return SF_BOOLEAN_FALSE;
    }
    // Get master token
    if (json_copy_string(&sf->master_token, data, master_token_str)) {
        log_error("No valid master token found in response");
        SET_SNOWFLAKE_ERROR(error, SF_ERROR_BAD_JSON, "Cannot find valid master token in response", SF_SQLSTATE_UNABLE_TO_CONNECT);
        return SF_BOOLEAN_FALSE;
    }

    return SF_BOOLEAN_TRUE;
}
