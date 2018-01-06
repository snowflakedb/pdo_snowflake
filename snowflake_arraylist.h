/*
 * Copyright (c) 2017-2018 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef PHP_PDO_SNOWFLAKE_ARRAYLIST_H
#define PHP_PDO_SNOWFLAKE_ARRAYLIST_H

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

ARRAY_LIST * STDCALL pdo_sf_array_list_init();
void STDCALL pdo_sf_array_list_deallocate(ARRAY_LIST *al);
void STDCALL pdo_sf_array_list_grow(ARRAY_LIST *al, size_t min_size);
void STDCALL pdo_sf_array_list_set(ARRAY_LIST *al, void *item, size_t index);
void *STDCALL pdo_sf_array_list_get(ARRAY_LIST *al, size_t index);

#ifdef  __cplusplus
}
#endif

#endif //PHP_PDO_SNOWFLAKE_ARRAYLIST_H
