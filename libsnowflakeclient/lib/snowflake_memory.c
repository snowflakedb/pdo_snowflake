/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include "snowflake_memory.h"
#include <snowflake_logger.h>
#include <pthread.h>

// Basic hashing function. Works well for memory addresses
#define sf_ptr_hash(p, t) (((unsigned long) (p) >> 3) & (sizeof (t)/sizeof ((t)[0]) - 1))
#define SF_ALLOC_MAP_SIZE 2048

pthread_mutex_t allocation_lock = PTHREAD_MUTEX_INITIALIZER;

static struct allocation {
    struct allocation *link;
    const void *ptr;
    size_t size;
    const char *file;
    int line;
} *alloc_map[SF_ALLOC_MAP_SIZE];

static struct allocation *alloc_find(const void *ptr) {
    struct allocation *alloc = alloc_map[sf_ptr_hash(ptr, alloc_map)];

    while (alloc && alloc->ptr != ptr) {
        alloc = alloc->link;
    }

    return alloc;
}

void alloc_insert(const void *ptr, size_t size, const char *file, int line) {
    struct allocation *alloc = malloc(sizeof(struct allocation));
    alloc->ptr = ptr;
    alloc->size = size;
    alloc->file = file;
    alloc->line = line;
    uint32 index = sf_ptr_hash(ptr, alloc_map);
    // Prepend
    alloc->link = alloc_map[index];
    alloc_map[index] = alloc;
}

void alloc_remove(const void *ptr) {
    uint32 index = sf_ptr_hash(ptr, alloc_map);
    struct allocation *prev = NULL;
    struct allocation *alloc = alloc_map[index];

    while (alloc && alloc->ptr != ptr) {
        prev = alloc;
        alloc = alloc->link;
    }

    // Remove alloc and set appropriate pointers
    if (prev && alloc) {
        prev->link = alloc->link;
    } else if (alloc) {
        alloc_map[index] = alloc->link;
        // Remove reference to next element in list
        alloc->link = NULL;
    }

    // Free alloc
    if (alloc) {
        free(alloc);
    }
}

void *sf_malloc(size_t size, const char *file, int line) {
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

    pthread_mutex_lock(&allocation_lock);
    alloc_insert(data, size, file, line);
    pthread_mutex_unlock(&allocation_lock);

    return data;
}

void *sf_calloc(size_t num, size_t size, const char *file, int line) {
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

    pthread_mutex_lock(&allocation_lock);
    alloc_insert(data, num * size, file, line);
    pthread_mutex_unlock(&allocation_lock);

    return data;
}

void *sf_realloc(void *ptr, size_t size, const char *file, int line) {
    struct allocation *alloc;
    // New pointer returned by realloc
    void *data = realloc(ptr, size);
    // If we could not allocate the needed data, exit
    if (data == NULL && size > 0) {
        log_fatal("Could not allocate %zu bytes of memory. Most likely out of memory. Exiting...", size);
        exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&allocation_lock);
    alloc = alloc_find(ptr);
    // If we don't find old entry then create new one
    if (alloc) {
        // No need to rehash since pointer is the same. Just update associated fields
        if (ptr == data) {
            alloc->size = size;
            alloc->file = file;
            alloc->line = line;
        } else {
            // Pointer is different, so we need to remove old entry, and create new entry
            alloc_remove(ptr);
            alloc_insert(data, size, file, line);
        }
    } else {
        alloc_insert(data, size, file, line);
    }
    pthread_mutex_unlock(&allocation_lock);

    return data;
}

void sf_free(void *ptr, const char *file, int line) {
    if (ptr) {
        pthread_mutex_lock(&allocation_lock);
        free(ptr);
        alloc_remove(ptr);
        pthread_mutex_unlock(&allocation_lock);
    }
}

void sf_alloc_map_to_log(sf_bool cleanup) {
    int i;
    struct allocation *alloc;
    struct allocation *link;
    pthread_mutex_lock(&allocation_lock);
    for (i = 0; i < SF_ALLOC_MAP_SIZE; i++) {
        if (alloc_map[i]) {
            alloc = alloc_map[i];
            while (alloc) {
                log_warn("Unallocated %zu bytes of memory at %p. Memory allocated in file %s at line %i",
                         alloc->size, (void *) alloc->ptr, alloc->file, alloc->line);
                link = alloc->link;
                if (cleanup) {
                    // Remove allocation if memory still exists
                    free(alloc);
                }
                alloc = link;
            }
        }
    }
    pthread_mutex_unlock(&allocation_lock);
}
