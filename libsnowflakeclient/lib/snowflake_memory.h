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

#define SF_MALLOC(s) sf_malloc(s, __FILE__, __LINE__)
#define SF_CALLOC(n, s) sf_calloc(n, s, __FILE__, __LINE__)
#define SF_REALLOC(p, s) sf_realloc(p, s, __FILE__, __LINE__)
#define SF_FREE(p) ((void) (sf_free(p, __FILE__, __LINE__), (p) = NULL))

void *sf_malloc(size_t size, const char *file, int line);
void *sf_calloc(size_t num, size_t size, const char *file, int line);
void *sf_realloc(void *ptr, size_t size, const char *file, int line);
void sf_free(void *ptr, const char *file, int line);

#ifdef __cplusplus
}
#endif

#endif //PDO_SNOWFLAKE_SNOWFLAKE_MEMORY_H