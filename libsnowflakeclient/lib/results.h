/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef PDO_SNOWFLAKE_RESULTS_H
#define PDO_SNOWFLAKE_RESULTS_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_WIN32)
#define STDCALL
#else
#define STDCALL __stdcall
#endif

#include "libsnowflakeclient/include/snowflake_client.h"
#include "basic_types.h"
#include "arraylist.h"
#include "cJSON.h"

SNOWFLAKE_TYPE string_to_snowflake_type(const char *string);
SNOWFLAKE_C_TYPE snowflake_to_c_type(SNOWFLAKE_TYPE type, int64 precision, int64 scale);
SNOWFLAKE_COLUMN_DESC ** set_description(const cJSON *rowtype);

#ifdef __cplusplus
}
#endif

#endif //PDO_SNOWFLAKE_RESULTS_H
