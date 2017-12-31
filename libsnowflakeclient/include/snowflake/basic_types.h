/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef SNOWFLAKE_BASIC_TYPES_H
#define SNOWFLAKE_BASIC_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_WIN32)
#define STDCALL
#else
#define STDCALL __stdcall
#endif

typedef char int8;
typedef unsigned char uint8;
typedef unsigned long int uint32;
typedef long int int32;
typedef unsigned long long int uint64;
typedef long long int int64;
typedef double float64;
typedef float float32;
typedef int8 sf_bool;

extern const int8 SF_BOOLEAN_TRUE;
extern const int8 SF_BOOLEAN_FALSE;

/**
 * Boolean data type string representation for Snowflake
 */
#define SF_BOOLEAN_INT_TRUE_STR "TRUE"
#define SF_BOOLEAN_INT_FALSE_STR "FALSE"

/**
 * Boolean data type string representation for results.
 *
 * This is mainly used by PHP PDO Snwoflake but should work fine
 * for other cases.
 */
#define SF_BOOLEAN_TRUE_STR "1"
#define SF_BOOLEAN_FALSE_STR ""

#ifdef __cplusplus
}
#endif

#endif /* SNOWFLAKE_BASIC_TYPES_H */
