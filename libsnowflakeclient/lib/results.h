/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef SNOWFLAKE_RESULTS_H
#define SNOWFLAKE_RESULTS_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_WIN32)
#define STDCALL
#else
#define STDCALL __stdcall
#endif

#include <snowflake_client.h>
#include "cJSON.h"

SF_TYPE string_to_snowflake_type(const char *string);
SF_C_TYPE snowflake_to_c_type(SF_TYPE type, int64 precision, int64 scale);
SF_TYPE c_type_to_snowflake(SF_C_TYPE c_type, SF_TYPE tsmode);
char *value_to_string(void *value, size_t len, SF_C_TYPE c_type);
SF_COLUMN_DESC ** set_description(const cJSON *rowtype);

#ifdef __cplusplus
}
#endif

#endif //SNOWFLAKE_RESULTS_H
