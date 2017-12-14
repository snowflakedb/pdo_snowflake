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
} SF_REQUEST_TYPE;

typedef enum sf_json_errors {
    SF_JSON_ERROR_NONE,
    SF_JSON_ERROR_ITEM_MISSING,
    SF_JSON_ERROR_ITEM_WRONG_TYPE,
    SF_JSON_ERROR_ITEM_NULL,
    SF_JSON_ERROR_OOM
} SF_JSON_ERROR;

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

#define JSON_ERROR_MSG(e, em, t) \
switch(e) \
{ \
    case SF_JSON_ERROR_ITEM_MISSING: (em) = #t " missing from JSON response"; break; \
    case SF_JSON_ERROR_ITEM_WRONG_TYPE: (em) = #t " is wrong type (expected a string)"; break; \
    case SF_JSON_ERROR_ITEM_NULL: (em) = #t " is null"; break; \
    case SF_JSON_ERROR_OOM: (em) = #t " caused an out of memory error"; break; \
    default: (em) = "Received unknown JSON error code trying to find " #t ; break; \
}

/*
 * Debug functions from curl example. Should update at somepoint, and possibly remove from header since these are private functions
 */
static void dump(const char *text, FILE *stream, unsigned char *ptr, size_t size, char nohex);
static int my_trace(CURL *handle, curl_infotype type, char *data, size_t size, void *userp);


cJSON *STDCALL create_auth_json_body(SF_CONNECT *sf,
                                     const char *application,
                                     const char *int_app_name,
                                     const char *int_app_version);
cJSON *STDCALL create_query_json_body(const char *sql_text, int64 sequence_id);
cJSON *STDCALL create_renew_session_json_body(const char *old_token);
struct curl_slist * STDCALL create_header_no_token();
struct curl_slist * STDCALL create_header_token(const char *header_token);
sf_bool STDCALL curl_post_call(SF_CONNECT *sf,
                               CURL *curl,
                               char *url,
                               struct curl_slist *header,
                               char *body,
                               cJSON **json,
                               SF_ERROR *error);
sf_bool STDCALL curl_get_call(SF_CONNECT *sf,
                              CURL *curl,
                              char *url,
                              struct curl_slist *header,
                              cJSON **json,
                              SF_ERROR *error);
uint32 decorrelate_jitter_next_sleep(DECORRELATE_JITTER_BACKOFF *djb, uint32 sleep);
char * STDCALL encode_url(CURL *curl,
                          const char *protocol,
                          const char *account,
                          const char *host,
                          const char *port,
                          const char *url,
                          URL_KEY_VALUE* vars,
                          int num_args,
                          SF_ERROR *error);
sf_bool is_string_empty(const char * str);
SF_JSON_ERROR STDCALL json_copy_bool(sf_bool *dest, cJSON *data, const char *item);
SF_JSON_ERROR STDCALL json_copy_int(int64 *dest, cJSON *data, const char *item);
SF_JSON_ERROR STDCALL json_copy_string(char **dest, cJSON *data, const char *item);
SF_JSON_ERROR STDCALL json_copy_string_no_alloc(char *dest, cJSON *data, const char *item, size_t dest_size);
SF_JSON_ERROR STDCALL json_detach_array_from_object(cJSON **dest, cJSON *data, const char *item);
SF_JSON_ERROR STDCALL json_detach_array_from_array(cJSON **dest, cJSON *data, int index);
SF_JSON_ERROR STDCALL json_detach_object_from_array(cJSON **dest, cJSON *data, int index);
size_t json_resp_cb(char *data, size_t size, size_t nmemb, RAW_JSON_BUFFER *raw_json);
sf_bool STDCALL http_perform(CURL *curl,
                             SF_REQUEST_TYPE request_type,
                             char *url,
                             struct curl_slist *header,
                             char *body,
                             cJSON **json,
                             int64 network_timeout,
                             sf_bool chunk_downloader,
                             SF_ERROR *error);
sf_bool STDCALL is_retryable_http_code(int32 code);
sf_bool STDCALL renew_session(CURL * curl, SF_CONNECT *sf, SF_ERROR *error);
sf_bool STDCALL request(SF_CONNECT *sf,
                        cJSON **json,
                        const char *url,
                        URL_KEY_VALUE* url_params,
                        int num_url_params,
                        char *body,
                        struct curl_slist *header,
                        SF_REQUEST_TYPE request_type,
                        SF_ERROR *error);
void STDCALL reset_curl(CURL *curl);
uint32 STDCALL retry_ctx_next_sleep(RETRY_CONTEXT *retry_ctx);
sf_bool STDCALL set_tokens(SF_CONNECT *sf,
                           cJSON *data,
                           const char *session_token_str,
                           const char *master_token_str,
                           SF_ERROR *error);

#ifdef __cplusplus
}
#endif

#endif //PDO_SNOWFLAKE_CONNECTION_H
