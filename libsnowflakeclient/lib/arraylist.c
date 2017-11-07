/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include "arraylist.h"
#include "snowflake_memory.h"

ARRAY_LIST *array_list_create() {
    ARRAY_LIST *al = (ARRAY_LIST *) calloc(1, sizeof(ARRAY_LIST));
    // Always initialize to 8
    al->size = 8;
    // Initialize array to NULL
    al->data = (void **) sf_calloc(al->size, sizeof(void *));
    return al;
}

void array_list_deallocate(ARRAY_LIST *al) {
    if (al != NULL) {
        sf_free(al->data);
    }
    sf_free(al);
}

void array_list_grow(ARRAY_LIST *al, size_t min_size) {
    size_t i;
    size_t new_size = al->size;
    while (new_size < min_size) {
        new_size *= 2;
    }
    al->data = (void **) sf_realloc(al->data, sizeof(void *) * new_size);
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
    al->data[index] = item;
}

void *array_list_get(ARRAY_LIST *al, size_t index) {
    return al->data[index];
}
