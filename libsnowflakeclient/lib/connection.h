/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef PDO_SNOWFLAKE_CONNECTION_H
#define PDO_SNOWFLAKE_CONNECTION_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_WIN32)
#define STDCALL
#else
#define STDCALL __stdcall
#endif

#include <stdio.h>
#include <curl/curl.h>
#include <snowflake_client.h>

typedef struct sf_raw_json_buffer {
    char *buffer;
    size_t size;
} RAW_JSON_BUFFER;

typedef struct sf_url_key_value {
    const char *key;
    char *value;
    const char *formatted_key;
    char *formatted_value;
    size_t key_size;
    size_t value_size;
} URL_KEY_VALUE;

struct data {
    char trace_ascii; /* 1 or 0 */
};

/*
 * Debug functions
 */
static void dump(const char *text, FILE *stream, unsigned char *ptr, size_t size, char nohex);
static int my_trace(CURL *handle, curl_infotype type, char *data, size_t size, void *userp);


cJSON *STDCALL create_auth_json_body(SNOWFLAKE *sf, const char *application, const char *int_app_name, const char *int_app_version);
cJSON *STDCALL create_query_json_body(char *sql_text, int64 sequence_id);
struct curl_slist * STDCALL create_header_no_token();
struct curl_slist * STDCALL create_header_token(char *header_token);
sf_bool STDCALL curl_post_call(CURL **curl, char *url, struct curl_slist *header, char *body, RAW_JSON_BUFFER *buffer, size_t (*writer)(char *, size_t, size_t, RAW_JSON_BUFFER *), struct data* config);
char * STDCALL encode_url(CURL *curl, const char *protocol, const char *host, const char *port, const char *url, URL_KEY_VALUE* vars, int num_args);
sf_bool STDCALL json_copy_bool(sf_bool *dest, cJSON *data, const char *item);
sf_bool STDCALL json_copy_int(int64 *dest, cJSON *data, const char *item);
sf_bool STDCALL json_copy_string(char **dest, cJSON *data, const char *item);
sf_bool STDCALL json_detach_array_from_object(cJSON **dest, cJSON *data, const char *item);
sf_bool STDCALL json_detach_array_from_array(cJSON **dest, cJSON *data, int index);
size_t json_resp_cb(char *data, size_t size, size_t nmemb, RAW_JSON_BUFFER *raw_json);

#ifdef __cplusplus
}
#endif

#endif //PDO_SNOWFLAKE_CONNECTION_H
