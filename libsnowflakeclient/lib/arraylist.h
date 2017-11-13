/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef PDO_SNOWFLAKE_ARRAYLIST_H
#define PDO_SNOWFLAKE_ARRAYLIST_H

#ifdef  __cplusplus
extern "C" {
#endif

#if !defined(_WIN32)
#define STDCALL
#else
#define STDCALL __stdcall
#endif

#include <stdlib.h>

typedef struct sf_array_list {
    void **data;
    size_t size;
    size_t used;
} ARRAY_LIST;

ARRAY_LIST * STDCALL array_list_init();
void STDCALL array_list_deallocate(ARRAY_LIST *al);
void STDCALL array_list_grow(ARRAY_LIST *al, size_t min_size);
void STDCALL array_list_set(ARRAY_LIST *al, void *item, size_t index);
void *STDCALL array_list_get(ARRAY_LIST *al, size_t index);

#ifdef  __cplusplus
}
#endif

#endif //PDO_SNOWFLAKE_ARRAYLIST_H
