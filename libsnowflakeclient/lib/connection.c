/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include "connection.h"
#include <string.h>
#include "basic_types.h"
#include "log.h"
#include "snowflake_memory.h"

#define curl_easier_escape(curl, string) curl_easy_escape(curl, string, 0)

int8 SF_BOOLEAN_TRUE = 1;
int8 SF_BOOLEAN_FALSE = 0;

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


cJSON *STDCALL create_auth_json_body(SNOWFLAKE *sf,
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

cJSON *STDCALL create_query_json_body(char *prepared_command, int64 sequence_id) {
    cJSON *body;
    // Create body
    body = cJSON_CreateObject();
    cJSON_AddStringToObject(body, "sqlText", prepared_command);
    cJSON_AddNumberToObject(body, "sequenceId", sequence_id);

    return body;
}

struct curl_slist * STDCALL create_header_no_token() {
    struct curl_slist * header = NULL;
    header = curl_slist_append(header, CONTENT_TYPE_APPLICATION_JSON);
    header = curl_slist_append(header, ACCEPT_TYPE_APPLICATION_SNOWFLAKE);
    header = curl_slist_append(header, C_API_USER_AGENT);
    return header;
}

struct curl_slist * STDCALL create_header_token(char *header_token) {
    struct curl_slist * header = NULL;
    header = curl_slist_append(header, header_token);
    header = curl_slist_append(header, CONTENT_TYPE_APPLICATION_JSON);
    header = curl_slist_append(header, ACCEPT_TYPE_APPLICATION_SNOWFLAKE);
    header = curl_slist_append(header, C_API_USER_AGENT);
    return header;
}

sf_bool STDCALL curl_post_call(CURL **curl,
                               char *url,
                               struct curl_slist *header,
                               char *body,
                               void *buffer,
                               size_t (*writer)(char *, size_t, size_t, void *),
                               struct data* config) {
    CURLcode res;
    CURL *conn = *curl;

    if (conn == NULL) {
        conn = curl_easy_init();
    }

    if(conn == NULL) {
        fprintf(stderr, "Failed to create CURL connection\n");
        return SF_BOOLEAN_FALSE;
    }

    //TODO set error buffer

    res = curl_easy_setopt(conn, CURLOPT_URL, url);
    if(res != CURLE_OK) {
        fprintf(stderr, "Failed to set URL [%s]\n", curl_easy_strerror(res));
        return SF_BOOLEAN_FALSE;
    }

    res = curl_easy_setopt(conn, CURLOPT_HTTPHEADER, header);
    if(res != CURLE_OK) {
        fprintf(stderr, "Failed to set header [%s]\n", curl_easy_strerror(res));
        return SF_BOOLEAN_FALSE;
    }

    res = curl_easy_setopt(conn, CURLOPT_POSTFIELDS, body);
    if(res != CURLE_OK) {
        fprintf(stderr, "Failed to set body [%s]\n", curl_easy_strerror(res));
        return SF_BOOLEAN_FALSE;
    }

    res = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, writer);
    if(res != CURLE_OK) {
        fprintf(stderr, "Failed to set writer [%s]\n", curl_easy_strerror(res));
        return SF_BOOLEAN_FALSE;
    }

    res = curl_easy_setopt(conn, CURLOPT_WRITEDATA, buffer);
    if(res != CURLE_OK) {
        fprintf(stderr, "Failed to set write data [%s]\n", curl_easy_strerror(res));
        return SF_BOOLEAN_FALSE;
    }

//    curl_easy_setopt(conn, CURLOPT_DEBUGFUNCTION, my_trace);
//    curl_easy_setopt(conn, CURLOPT_DEBUGDATA, config);
//
//    /* the DEBUGFUNCTION has no effect until we enable VERBOSE */
//    curl_easy_setopt(conn, CURLOPT_VERBOSE, 1);

    log_info("Running curl call");
    res = curl_easy_perform(conn);
    /* Check for errors */
    if(res != CURLE_OK) {
        log_error("curl_easy_perform() failed: %s", curl_easy_strerror(res));
        return SF_BOOLEAN_FALSE;
    }

    return SF_BOOLEAN_TRUE;
}

char * encode_url(CURL *curl, const char *protocol, const char *host, const char *port, const char *url, URL_KEY_VALUE* vars, int num_args) {
    int i;
    const char *format = "%s://%s:%s%s";
    char *encoded_url = NULL;
    // Size used for the url format
    size_t base_url_size;
    // Size used to determine buffer size
    size_t encoded_url_size;
    int bytes_written;
    // Null terminator plus format contains 4 hardcoded characters
    base_url_size = 5 + strlen(protocol) + strlen(host) + strlen(port) + strlen(url);
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
    bytes_written = snprintf(encoded_url, base_url_size, format, protocol, host, port, url);

    if (bytes_written < 0 || bytes_written >= encoded_url_size) {
        log_warn("Encoded url was not properly constructed. Expected size: %zu     Actual Size: %i",
                 encoded_url_size, bytes_written);
        SF_FREE(encoded_url);
        encoded_url = NULL;
        goto cleanup;
    }

    // Add encoded URL parameters to encoded_url buffer
    for (i = 0; i < num_args; i++) {
        strncat(encoded_url, vars[i].formatted_key, vars[i].key_size);
        strncat(encoded_url, vars[i].formatted_value, vars[i].value_size);
    }

    log_debug("Here is constructed url: %s", encoded_url);
    log_trace("Encoded Base URL sizes. Expected size: %zu     Actual Size: %i", base_url_size, bytes_written);

cleanup:
    // Free created memory
    for (i = 0; i < num_args; i++) {
        SF_FREE(vars[i].formatted_value);
    }

    return encoded_url;
}

sf_bool json_copy_string(char **dest, cJSON *data, const char *item) {
    size_t blob_size;
    cJSON *blob = cJSON_GetObjectItem(data, item);
    if (cJSON_IsString(blob)) {
        blob_size = strlen(blob->valuestring) + 1;
        SF_FREE(*dest);
        *dest = (char *) SF_CALLOC(1, blob_size);
        strncpy(*dest, blob->valuestring, blob_size);
        log_debug("Found item and value; %s: %s", item, *dest);
        return SF_BOOLEAN_TRUE;
    }

    return SF_BOOLEAN_FALSE;
}

sf_bool json_copy_bool(sf_bool *dest, cJSON *data, const char *item) {
    cJSON *blob = cJSON_GetObjectItem(data, item);
    if (cJSON_IsBool(blob)) {
        *dest = cJSON_IsTrue(blob) ? SF_BOOLEAN_TRUE : SF_BOOLEAN_FALSE;
        log_debug("Found item and value; %s: %i", item, *dest);
        return SF_BOOLEAN_TRUE;
    }

    return SF_BOOLEAN_FALSE;
}

sf_bool json_copy_int(int64 *dest, cJSON *data, const char *item) {
    cJSON *blob = cJSON_GetObjectItem(data, item);
    if (cJSON_IsNumber(blob)) {
        *dest = (int64) blob->valuedouble;
        log_debug("Found item and value; %s: %i", item, *dest);
        return SF_BOOLEAN_TRUE;
    }

    return SF_BOOLEAN_FALSE;
}

sf_bool json_detach_array_from_object(cJSON **dest, cJSON *data, const char *item) {
    cJSON *blob = cJSON_DetachItemFromObject(data, item);
    if (cJSON_IsArray(blob)) {
        if (*dest) {
            cJSON_Delete(*dest);
        }
        *dest = blob;
        log_debug("Found array item: %s", item);
        return SF_BOOLEAN_TRUE;
    }

    return SF_BOOLEAN_FALSE;
}

sf_bool json_detach_array_from_array(cJSON **dest, cJSON *data, int index) {
    cJSON *blob = cJSON_DetachItemFromArray(data, index);
    if (blob && cJSON_IsArray(blob)) {
        if (*dest) {
            cJSON_Delete(*dest);
        }
        *dest = blob;
        log_debug("Found array item at index: %s", index);
        return SF_BOOLEAN_TRUE;
    }

    return SF_BOOLEAN_FALSE;
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
