/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef PDO_SNOWFLAKE_CHUNK_DOWNLOADER_H
#define PDO_SNOWFLAKE_CHUNK_DOWNLOADER_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_WIN32)
#define STDCALL
#else
#define STDCALL __stdcall
#endif

#include <pthread.h>
#include <curl/curl.h>
#include <snowflake_client.h>

typedef struct sf_queue_item {
    char *url;
    int64 row_count;
    cJSON *chunk;
} SF_QUEUE_ITEM;

struct sf_chunk_downloader {
    // Threads
    pthread_t *threads;
    uint64 thread_count;

    // "Queue" locks
    pthread_mutex_t queue_lock;
    pthread_cond_t producer_cond;
    pthread_cond_t consumer_cond;

    // A "queue" that is actually just a locked array
    SF_QUEUE_ITEM* queue;

    // Queue attributes
    uint64 producer_head;
    uint64 consumer_head;
    uint64 queue_size;

    // Chunk downloader connection attributes
    char *qrmk;
    struct curl_slist *chunk_headers;

    // Error/shutdown flags
    sf_bool is_shutdown;
    sf_bool has_error;

    // Chunk downloader attribute read-write lock. If you need to acquire both the queue_lock and attr_lock,
    // ALWAYS acquire the queue_lock first, otherwise we can deadlock
    pthread_rwlock_t attr_lock;

    // Snowflake statement error
    SF_ERROR *sf_error;
};

SF_CHUNK_DOWNLOADER *STDCALL chunk_downloader_init(const char *qrmk,
                                                   cJSON* chunk_headers,
                                                   cJSON *chunks,
                                                   uint64 thread_count,
                                                   uint64 fetch_slots,
                                                   SF_ERROR *sf_error);
sf_bool STDCALL chunk_downloader_term(SF_CHUNK_DOWNLOADER *chunk_downloader);
static void *chunk_downloader_thread(void *downloader);
sf_bool get_shutdown_or_error(SF_CHUNK_DOWNLOADER *chunk_downloader);
sf_bool get_shutdown(SF_CHUNK_DOWNLOADER *chunk_downloader);
void set_shutdown(SF_CHUNK_DOWNLOADER *chunk_downloader, sf_bool value);
sf_bool get_error(SF_CHUNK_DOWNLOADER *chunk_downloader);
void set_error(SF_CHUNK_DOWNLOADER *chunk_downloader, sf_bool value);


#ifdef __cplusplus
}
#endif

#endif //PDO_SNOWFLAKE_CHUNK_DOWNLOADER_H
