/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef PDO_SNOWFLAKE_ERROR_H
#define PDO_SNOWFLAKE_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_WIN32)
#define STDCALL
#else
#define STDCALL __stdcall
#endif

#include <snowflake_client.h>

#define SET_SNOWFLAKE_ERROR(e, ec, m, s) set_snowflake_error(e, ec, m, s, __FILE__, __LINE__)

void STDCALL set_snowflake_error(SNOWFLAKE_ERROR *error,
                                  SNOWFLAKE_ERROR_CODE error_code,
                                  const char *msg,
                                  const char *sfqid,
                                  const char *file,
                                  int line);
void STDCALL clear_snowflake_error(SNOWFLAKE_ERROR *error);

#ifdef __cplusplus
}
#endif

#endif //PDO_SNOWFLAKE_ERROR_H
