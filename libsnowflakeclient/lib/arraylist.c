/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include "arraylist.h"
#include "snowflake_memory.h"

ARRAY_LIST *array_list_init() {
    ARRAY_LIST *al = (ARRAY_LIST *) calloc(1, sizeof(ARRAY_LIST));
    // No spots are used yet
    al->used = 0;
    // Always initialize to 8
    al->size = 8;
    // Initialize array to NULL
    al->data = (void **) SF_CALLOC(al->size, sizeof(void *));
    return al;
}

void array_list_deallocate(ARRAY_LIST *al) {
    if (al != NULL) {
        SF_FREE(al->data);
    }
    SF_FREE(al);
}

void array_list_grow(ARRAY_LIST *al, size_t min_size) {
    size_t i;
    size_t new_size = al->size;
    while (new_size < min_size) {
        new_size *= 2;
    }
    al->data = (void **) SF_REALLOC(al->data, sizeof(void *) * new_size);
    // Initialize new memory to NULL
    for (i = al->size; i < new_size; i++) {
        al->data[i] = NULL;
    }
    al->size = new_size;
}

void array_list_set(ARRAY_LIST *al, void *item, size_t index) {
    if (al->size < index) {
        array_list_grow(al, index);
    }
    // If element we are writing to is NULL, we want to increment 'used'.
    // Otherwise we are writing to a spot that already contains an element
    if (al->data[index] == NULL) {
        al->used++;
    }
    al->data[index] = item;
}

void *array_list_get(ARRAY_LIST *al, size_t index) {
    return al->data[index];
}