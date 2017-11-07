/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef PDO_SNOWFLAKE_SNOWFLAKE_MEMORY_H
#define PDO_SNOWFLAKE_SNOWFLAKE_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_WIN32)
#define STDCALL
#else
#define STDCALL __stdcall
#endif

#include <stdlib.h>

void *sf_malloc(size_t size);
void *sf_calloc(size_t num, size_t size);
void *sf_realloc(void *ptr, size_t size);
void sf_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif //PDO_SNOWFLAKE_SNOWFLAKE_MEMORY_H
