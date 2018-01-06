/*
 * Copyright (c) 2017-2018 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef SNOWFLAKE_CONNECTION_H
#define SNOWFLAKE_CONNECTION_H

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
#include <snowflake/client.h>
#include "cJSON.h"
#include "arraylist.h"

/**
 * Request type
 */
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

/**
 * Error types returned by cJSON convenience functions.
 */
typedef enum sf_json_errors {
    /** No error */
    SF_JSON_ERROR_NONE,

    /** JSON item is not found in provieded blob. */
    SF_JSON_ERROR_ITEM_MISSING,

    /** JSON item is the wrong type. Try a different convenience function. */
    SF_JSON_ERROR_ITEM_WRONG_TYPE,

    /** JSON item value is null */
    SF_JSON_ERROR_ITEM_NULL,

    /** Out of Memory */
    SF_JSON_ERROR_OOM
} SF_JSON_ERROR;

/**
 * Dynamically growing char buffer to hold retrieved in cURL call.
 */
typedef struct sf_raw_json_buffer {
    // Char buffer
    char *buffer;
    // Number of characters in char buffer
    size_t size;
} RAW_JSON_BUFFER;

/**
 * URL Parameter struct used to construct an encoded URL.
 */
typedef struct sf_url_key_value {
    // URL param key
    const char *key;
    // URL param value
    char *value;
    // URL param formatted key
    const char *formatted_key;
    // URL param formatted value
    char *formatted_value;
    // Formatted key size
    size_t key_size;
    // Formatted value size
    size_t value_size;
} URL_KEY_VALUE;

/**
 * Used to keep track of min and max backoff time for a connection retry
 */
typedef struct sf_decorrelate_jitter_backoff {
    // Minimum backoff time
    uint32 base;
    // Maximum backoff time
    uint32 cap;
} DECORRELATE_JITTER_BACKOFF;

/**
 * Connection retry struct to keep track of retry status
 */
typedef struct sf_retry_context {
    // Number of retries
    uint64 retry_count;
    // Retry timeout in number of seconds.
    uint64 retry_timeout;
    // Time to sleep in seconds
    uint32 sleep_time;
    // Decorrelate Jitter is used to determine sleep time
    DECORRELATE_JITTER_BACKOFF *djb;
} RETRY_CONTEXT;

/**
 * Debug struct from curl example. Need to update at somepoint.
 */
struct data {
    char trace_ascii; /* 1 or 0 */
};

/**
 * Macro to get a custom error message to pass to the Snowflake Error object.
 */
#define JSON_ERROR_MSG(e, em, t) \
switch(e) \
{ \
    case SF_JSON_ERROR_ITEM_MISSING: (em) = #t " missing from JSON response"; break; \
    case SF_JSON_ERROR_ITEM_WRONG_TYPE: (em) = #t " is wrong type (expected a string)"; break; \
    case SF_JSON_ERROR_ITEM_NULL: (em) = #t " is null"; break; \
    case SF_JSON_ERROR_OOM: (em) = #t " caused an out of memory error"; break; \
    default: (em) = "Received unknown JSON error code trying to find " #t ; break; \
}

/**
 * Creates connection authorization body as a cJSON blob. cJSON blob must be freed by the caller using cJSON_Delete.
 *
 * @param sf Snowflake Connection object. Uses account, user, and password from Connection struct.
 * @param application Application type.
 * @param int_app_name Client ID.
 * @param int_app_version Client App Version. Used to ensure we reject unsupported clients.
 * @param timezone Timezone
 * @param autocommit Wheter autocommit is enabled.
 * @return Authorization cJSON Body.
 */
cJSON *STDCALL create_auth_json_body(SF_CONNECT *sf, const char *application, const char *int_app_name,
                                     const char *int_app_version, const char* timezone, sf_bool autocommit);

/**
 * Creates a cJSON blob used to execute queries. cJSON blob must be freed by the caller using cJSON_Delete.
 *
 * @param sql_text The sql query to send to Snowflake
 * @param sequence_id Sequence ID from the Snowflake Connection object.
 * @return Query cJSON Body.
 */
cJSON *STDCALL create_query_json_body(const char *sql_text, int64 sequence_id);

/**
 * Creates a cJSON blob that is used to renew a session with Snowflake. cJSON blob must be freed by the caller using
 * cJSON_Delete.
 *
 * @param old_token Expired session token from Snowflake Connection object.
 * @return Renew Session cJSON Body.
 */
cJSON *STDCALL create_renew_session_json_body(const char *old_token);

/**
 * Creates a header to give to cURL to connect to Snowflake. Must be freed by the caller.
 *
 * @return cURL header list.
 */
struct curl_slist * STDCALL create_header_no_token();

/**
 * Creates a header to give to cURL to issue commands to Snowflake once a connection has been established. Must be
 * freed by the caller.
 *
 * @param header_token
 * @return cURL header list
 */
struct curl_slist * STDCALL create_header_token(const char *header_token);

/**
 * Used to issue a cURL POST call to Snowflake. Includes support for ping-pong and renew session. If the request was
 * successful, we return 1, otherwise 0
 *
 * @param sf Snowflake Connection object. Used to get network timeout and pass to renew session
 * @param curl cURL instance that is currently in use for the request
 * @param url URL to send the request to
 * @param header Header passed to cURL for use in the request
 * @param body Body passed to cURL for use in the request
 * @param json Reference to a cJSON pointer that is used to store the JSON response upon a successful request
 * @param error Reference to the Snowflake Error object to set an error if one occurs
 * @return Success/failure status of post call. 1 = Success; 0 = Failure
 */
sf_bool STDCALL curl_post_call(SF_CONNECT *sf, CURL *curl, char *url, struct curl_slist *header, char *body,
                               cJSON **json, SF_ERROR *error);

/**
 * Used to issue a cURL GET call to Snowflake. Includes support for renew session. If the request was successful,
 * we return 1, otherwise 0
 *
 * @param sf Snowflake Connection object. Used to get network timeout and pass to renew session
 * @param curl cURL instance that is currently in use for the request
 * @param url URL to send the request to
 * @param header Header passed to cURL for use in the request
 * @param json Reference to a cJSON pointer that is used to store the JSON response upon a successful request
 * @param error Reference to the Snowflake Error object to set an error if one occurs
 * @return Success/failure status of get call. 1 = Success; 0 = Failure
 */
sf_bool STDCALL curl_get_call(SF_CONNECT *sf, CURL *curl, char *url, struct curl_slist *header, cJSON **json,
                              SF_ERROR *error);

/**
 * Used to determine the sleep time during the next backoff caused by request failure.
 *
 * @param djb Decorrelate Jitter Backoff object used to determine min and max backoff time.
 * @param sleep Duration of last sleep in seconds.
 * @return Number of seconds to sleep.
 */
uint32 decorrelate_jitter_next_sleep(DECORRELATE_JITTER_BACKOFF *djb, uint32 sleep);

/**
 * Creates a URL that is safe to use with cURL. Caller must free the memory associated with the encoded URL.
 *
 * @param curl cURL object for the request.
 * @param protocol Protocol to use in the request. Either HTTP or HTTPS.
 * @param account Snowflake account name. This should be the account of the user.
 * @param host Host to connect to. Used when connecting to different Snowflake deployments
 * @param port Port to connect to. Used when connecting to a non-conventional port.
 * @param url URL path to use.
 * @param vars URL parameters to add to the encoded URL.
 * @param num_args Number of URL parameters.
 * @param error Reference to the Snowflake Error object to set an error if one occurs.
 * @return Returns a pointer to a string which is the the encoded URL.
 */
char * STDCALL encode_url(CURL *curl, const char *protocol, const char *account, const char *host, const char *port,
                          const char *url, URL_KEY_VALUE* vars, int num_args, SF_ERROR *error);

/**
 * Determines if a string is empty by checking if the passed in string is NULL or contains a null terminator as its
 * first character
 *
 * @param str String to check
 * @return True (1) if empty, False (0) if not empty
 */
sf_bool is_string_empty(const char * str);

/**
 * A convenience function that copies an item value, specified by the item field, to the dest field if the item
 * exists, isn't null and is the right type.
 *
 * @param dest A reference to the location of a sf_bool variable where the JSON item value should be copied to.
 * @param data A reference to the cJSON object to find the value specified by item.
 * @param item The name (key) of the object item that you are looking for.
 * @return Returns an SF_JSON_ERROR return code; can be processed or ignored by the caller.
 */
SF_JSON_ERROR STDCALL json_copy_bool(sf_bool *dest, cJSON *data, const char *item);

/**
 * A convenience function that copies an item value, specified by the item field, to the dest field if the item
 * exists, isn't null and is the right type.
 *
 * @param dest A reference to the location of a 64-bit int where the JSON item value should be copied to.
 * @param data A reference to the cJSON object to find the value specified by item.
 * @param item The name (key) of the object item that you are looking for.
 * @return Returns an SF_JSON_ERROR return code; can be processed or ignored by the caller.
 */
SF_JSON_ERROR STDCALL json_copy_int(int64 *dest, cJSON *data, const char *item);

/**
 * A convenience function that copies an item value, specified by the item field, to the dest field if the item
 * exists, isn't null and is the right type. The caller is responsible for freeing the memory referenced by dest.
 *
 * @param dest A reference to a char pointer where the JSON string pointer should be copied to. The pointer referenced
 *             by dest needs to be freed by the caller.
 * @param data A reference to the cJSON object to find the value specified by item.
 * @param item The name (key) of the object item that you are looking for.
 * @return Returns an SF_JSON_ERROR return code; can be processed or ignored by the caller.
 */
SF_JSON_ERROR STDCALL json_copy_string(char **dest, cJSON *data, const char *item);

/**
 * A convenience function that copies an item value, specified by the item field, to the string buffer specified by
 * dest field if the item exists, isn't null and is the right type.
 *
 * @param dest A reference to the string buffer where the JSON item value should be copied to.
 * @param data A reference to the cJSON object to find the value specified by item.
 * @param item The name (key) of the object item that you are looking for.
 * @param dest_size The size of the string buffer
 * @return Returns an SF_JSON_ERROR return code; can be processed or ignored by the caller.
 */
SF_JSON_ERROR STDCALL json_copy_string_no_alloc(char *dest, cJSON *data, const char *item, size_t dest_size);

/**
 * A convenience function that detaches a cJSON array from a cJSON object blob and sets the reference to the array in
 * dest. Checks to make sure that the item exists, isn't null and is the right type.
 *
 * @param dest A reference to the cJSON pointer where the cJSON array pointer should be copied to.
 * @param data A reference to the cJSON object to find the array specified by item.
 * @param item The name (key) of the object item that you are looking for.
 * @return Returns an SF_JSON_ERROR return code; can be processed or ignored by the caller.
 */
SF_JSON_ERROR STDCALL json_detach_array_from_object(cJSON **dest, cJSON *data, const char *item);

/**
 * A convenience function that detaches a cJSON array from a cJSON array and sets the reference to the array in
 * dest. Checks to make sure that the item exists, isn't null and is the right type.
 *
 * @param dest A reference to the cJSON pointer where the cJSON array pointer should be copied to.
 * @param data A reference to the cJSON object to find the array specified by item.
 * @param item The name (key) of the object item that you are looking for.
 * @return Returns an SF_JSON_ERROR return code; can be processed or ignored by the caller.
 */
SF_JSON_ERROR STDCALL json_detach_array_from_array(cJSON **dest, cJSON *data, int index);

/**
 * A convenience function that detaches a cJSON object from a cJSON array and sets the reference to the array in
 * dest. Checks to make sure that the item exists, isn't null and is the right type.
 *
 * @param dest A reference to the cJSON pointer where the cJSON object pointer should be copied to.
 * @param data A reference to the cJSON object to find the array specified by item.
 * @param item The name (key) of the object item that you are looking for.
 * @return Returns an SF_JSON_ERROR return code; can be processed or ignored by the caller.
 */
SF_JSON_ERROR STDCALL json_detach_object_from_array(cJSON **dest, cJSON *data, int index);

/**
 * Returns all the keys in a JSON object. To save memory, the keys in the array list are references
 * to the keys in the cJSON structs. DO NOT free these keys in the arraylist. Once you delete the cJSON
 * object, you must also destroy the arraylist containing the object keys.
 *
 * @param item A cJSON object that will not be altered in the function.
 * @return An arraylist containing all the keys in the object. This arraylist must be freed by the caller at some point.
 */
ARRAY_LIST *json_get_object_keys(const cJSON const *item);

/**
 * A write callback function to use to write the response text received from the cURL response. The raw JSON buffer
 * will grow in size until
 *
 * @param data The data to copy in the buffer.
 * @param size The size (in bytes) of each data member.
 * @param nmemb The number of data members.
 * @param raw_json The Raw JSON Buffer object that grows in size to copy multiple writes for a single cURL call.
 * @return The number of bytes copied into the buffer.
 */
size_t json_resp_cb(char *data, size_t size, size_t nmemb, RAW_JSON_BUFFER *raw_json);

/**
 * Performs an HTTP request with retry.
 *
 * @param curl The cURL object to use in the request.
 * @param request_type The type of HTTP request.
 * @param url The fully qualified URL to use for the HTTP request.
 * @param header The header to use for the HTTP request.
 * @param body The body to send over the HTTP request. If running GET request, set this to NULL.
 * @param json A reference to a cJSON pointer where we should store a successful request.
 * @param network_timeout The network request timeout to use for each request try.
 * @param chunk_downloader A boolean value determining whether or not we are running this request from the chunk
 *                         downloader. Each chunk that we download from AWS is invalid JSON so we need to add an
 *                         opening square bracket at the beginning of the text buffer and a closing square bracket
 *                         at the end of the text buffer.
 * @param error Reference to the Snowflake Error object to set an error if one occurs.
 * @return Success/failure status of http request call. 1 = Success; 0 = Failure
 */
sf_bool STDCALL http_perform(CURL *curl, SF_REQUEST_TYPE request_type, char *url, struct curl_slist *header,
                             char *body, cJSON **json, int64 network_timeout, sf_bool chunk_downloader, SF_ERROR *error);

/**
 * Returns true if HTTP code is retryable, false otherwise.
 *
 * @param code The HTTP code to test.
 * @return Retryable/Non-retryable. 1 = Retryable; 0 = Non-retryable
 */
sf_bool STDCALL is_retryable_http_code(int32 code);

/**
 * Renews a session once the session token has expired.
 *
 * @param curl cURL object to use for the renew session request to Snowflake
 * @param sf The Snowflake Connection object to use for connection details.
 * @param error Reference to the Snowflake Error object to set an error if one occurs.
 * @return Success/failure status of session renewal. 1 = Success; 0 = Failure
 */
sf_bool STDCALL renew_session(CURL * curl, SF_CONNECT *sf, SF_ERROR *error);

/**
 * Runs a request to Snowflake. Encodes the URL and creates the cURL object that is used for the request.
 *
 * @param sf The Snowflake Connection object to use for connection details.
 * @param json A reference to a cJSON pointer. Holds the response of the request.
 * @param url The URL path for the request. A full URL will be constructed using this path.
 * @param url_params URL parameters to add to the encoded URL.
 * @param num_url_params Number of URL parameters.
 * @param body JSON body text to send as part of the request. If running a GET request, set to NULL.
 * @param header Header to use in the request.
 * @param request_type Type of request.
 * @param error Reference to the Snowflake Error object to set an error if one occurs.
 * @return Success/failure status of request. 1 = Success; 0 = Failure
 */
sf_bool STDCALL request(SF_CONNECT *sf, cJSON **json, const char *url, URL_KEY_VALUE* url_params, int num_url_params,
                        char *body, struct curl_slist *header, SF_REQUEST_TYPE request_type, SF_ERROR *error);

/**
 * Resets curl instance.
 *
 * @param curl cURL object.
 */
void STDCALL reset_curl(CURL *curl);

/**
 * Determines next sleep duration for request retry. Sets new sleep duration value in Retry Context.
 *
 * @param retry_ctx Retry Context object. Used to determine next sleep.
 * @return Returns number of seconds to sleep.
 */
uint32 STDCALL retry_ctx_next_sleep(RETRY_CONTEXT *retry_ctx);

/**
 * Convenience function to set tokens in Snowflake Connect object from cJSON blob. Returns success/failure.
 *
 * @param sf The Snowflake Connection object to update the session/master keys of.
 * @param data cJSON blob containing new keys.
 * @param session_token_str Session token JSON key.
 * @param master_token_str Master token JSON key.
 * @param error Reference to the Snowflake Error object to set an error if one occurs.
 * @return Success/failure of key setting. 1 = Success; 0 = Failure
 */
sf_bool STDCALL set_tokens(SF_CONNECT *sf, cJSON *data, const char *session_token_str, const char *master_token_str,
                           SF_ERROR *error);

#ifdef __cplusplus
}
#endif

#endif //SNOWFLAKE_CONNECTION_H
