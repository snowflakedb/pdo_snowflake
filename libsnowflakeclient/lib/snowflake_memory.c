/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include "snowflake_memory.h"
#include "log.h"
#include "basic_types.h"

// Basic hashing function. Works well for memory addresses
#define hash(p, t) (((unsigned long) (p) >> 3) & (sizeof (t)/sizeof ((t)[0]) - 1))

static struct allocation {
    struct allocation *link;
    const void *ptr;
    size_t size;
    const char *file;
    int line;
} *alloc_map[2048];

static struct allocation *alloc_find(const void *ptr) {
    struct allocation *alloc = alloc_map[hash(ptr, alloc_map)];

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
    uint32 index = hash(ptr, alloc_map);
    // Prepend
    alloc->link = alloc_map[index];
    alloc_map[index] = alloc;
}

void alloc_remove(const void *ptr) {
    uint32 index = hash(ptr, alloc_map);
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

    alloc_insert(data, size, file, line);

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

    alloc_insert(data, num * size, file, line);

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

    return data;
}

void sf_free(void *ptr, const char *file, int line) {
    if (ptr) {
        free(ptr);
    }
}
