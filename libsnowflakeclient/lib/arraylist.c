/*
 * Copyright (c) 2017-2018 Snowflake Computing, Inc. All rights reserved.
 */

#include "arraylist.h"
#include "memory.h"

ARRAY_LIST *sf_array_list_init() {
    ARRAY_LIST *al = (ARRAY_LIST *) calloc(1, sizeof(ARRAY_LIST));
    // No spots are used yet
    al->used = 0;
    // Always initialize to 8
    al->size = 8;
    // Initialize array to NULL
    al->data = (void **) SF_CALLOC(al->size, sizeof(void *));
    return al;
}

void sf_array_list_deallocate(ARRAY_LIST *al) {
    if (al != NULL) {
        SF_FREE(al->data);
    }
    SF_FREE(al);
}

void sf_array_list_grow(ARRAY_LIST *al, size_t min_size) {
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

void sf_array_list_set(ARRAY_LIST *al, void *item, size_t index) {
    if (al->size < index) {
        sf_array_list_grow(al, index);
    }
    // If element we are writing to is NULL and item is not NULL, we want to increment 'used'.
    // Otherwise we are writing to a spot that already contains an element
    if (!al->data[index] && item) {
        al->used++;
    } else if (al->data[index] && !item) {
        // If this element exists and item is NULL, then we are deleting from the arraylist
        // and so we decrement used
        al->used--;
    }
    al->data[index] = item;
}

void *sf_array_list_get(ARRAY_LIST *al, size_t index) {
    return al->data[index];
}
