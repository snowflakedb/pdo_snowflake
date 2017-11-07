/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include "snowflake_memory.h"
#include "log.h"

void *sf_malloc(size_t size) {
    // If size is 0, we should return a NULL pointer instead of exiting.
    if (size == 0) {
        return NULL;
    }
    void *data = malloc(size);
    // If we could not allocate the needed data, exit
    if (data == NULL) {
        log_fatal("Could not allocate %zu bytes of memory. Most likely out of memory. Exiting...", size);
        exit(EXIT_FAILURE);
    }
    return data;
}

void *sf_calloc(size_t num, size_t size) {
    // If size or num is 0, we should return a NULL pointer instead of exiting.
    if (size == 0 || num == 0) {
        return NULL;
    }
    void *data = calloc(num, size);
    // If we could not allocate the needed data, exit
    if (data == NULL) {
        log_fatal("Could not allocate %zu bytes of memory. Most likely out of memory. Exiting...", (num * size));
        exit(EXIT_FAILURE);
    }
    return data;
}

void *sf_realloc(void *ptr, size_t size) {
    // New pointer returned by realloc
    void *data = realloc(ptr, size);
    // If we could not allocate the needed data, exit
    if (data == NULL && size > 0) {
        log_fatal("Could not allocate %zu bytes of memory. Most likely out of memory. Exiting...", size);
        exit(EXIT_FAILURE);
    }
    return data;
}

void sf_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    free(ptr);
}
