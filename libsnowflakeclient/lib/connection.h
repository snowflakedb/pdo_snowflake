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
#include <stdlib.h>
#include <curl/curl.h>
#include <snowflake_client.h>

typedef enum sf_request_type {
            /** not handling any request right now */
            NONE_REQUEST_TYPE,

            /** we are doing a http get */
            GET_REQUEST_TYPE,

            /** we are doing a http put */
            PUT_REQUEST_TYPE,

            /** we are doing a http post */
            POST_REQUEST_TYPE,

            /** we are doing a http delete */
            DELETE_REQUEST_TYPE,
} SNOWFLAKE_REQUEST_TYPE;

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

typedef struct sf_decorrelate_jitter_backoff {
    uint32 base;
    uint32 cap;
} DECORRELATE_JITTER_BACKOFF;

typedef struct sf_retry_context {
    uint64 retry_count;
    uint64 retry_timeout;
    uint32 sleep_time;
    DECORRELATE_JITTER_BACKOFF *djb;
} RETRY_CONTEXT;

struct data {
    char trace_ascii; /* 1 or 0 */
};

/*
 * Debug functions from curl example. Should update at somepoint, and possibly remove from header since these are private functions
 */
static void dump(const char *text, FILE *stream, unsigned char *ptr, size_t size, char nohex);
static int my_trace(CURL *handle, curl_infotype type, char *data, size_t size, void *userp);


cJSON *STDCALL create_auth_json_body(SNOWFLAKE *sf,
                                     const char *application,
                                     const char *int_app_name,
                                     const char *int_app_version);
cJSON *STDCALL create_query_json_body(char *sql_text, int64 sequence_id);
struct curl_slist * STDCALL create_header_no_token();
struct curl_slist * STDCALL create_header_token(const char *header_token);
sf_bool STDCALL curl_post_call(SNOWFLAKE *sf, CURL *curl, char *url, struct curl_slist *header, char *body,
                               cJSON **json);
sf_bool STDCALL curl_get_call(SNOWFLAKE *sf, CURL *curl, char *url, struct curl_slist *header, cJSON **json);
uint32 decorrelate_jitter_next_sleep(DECORRELATE_JITTER_BACKOFF *djb, uint32 sleep);
char * STDCALL encode_url(CURL *curl,
                          const char *protocol,
                          const char *host,
                          const char *port,
                          const char *url,
                          URL_KEY_VALUE* vars,
                          int num_args);
sf_bool STDCALL json_copy_bool(sf_bool *dest, cJSON *data, const char *item);
sf_bool STDCALL json_copy_int(int64 *dest, cJSON *data, const char *item);
sf_bool STDCALL json_copy_string(char **dest, cJSON *data, const char *item);
sf_bool STDCALL json_detach_array_from_object(cJSON **dest, cJSON *data, const char *item);
sf_bool STDCALL json_detach_array_from_array(cJSON **dest, cJSON *data, int index);
size_t json_resp_cb(char *data, size_t size, size_t nmemb, RAW_JSON_BUFFER *raw_json);
sf_bool STDCALL http_perform(SNOWFLAKE *sf, CURL *curl, SNOWFLAKE_REQUEST_TYPE request_type, char *url, struct curl_slist *header, char *body, cJSON **json);
sf_bool STDCALL is_retryable_http_code(int32 code);
sf_bool STDCALL request(SNOWFLAKE *sf,
                        cJSON **json,
                        const char *url,
                        URL_KEY_VALUE* url_params,
                        int num_url_params,
                        char *body,
                        SNOWFLAKE_REQUEST_TYPE request_type);
void STDCALL reset_curl(CURL *curl);
uint32 STDCALL retry_ctx_next_sleep(RETRY_CONTEXT *retry_ctx);

#ifdef __cplusplus
}
#endif

#endif //PDO_SNOWFLAKE_CONNECTION_H
