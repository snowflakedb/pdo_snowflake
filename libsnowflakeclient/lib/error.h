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

#define SET_SNOWFLAKE_ERROR(e, ec, m, sqlstate) set_snowflake_error(e, ec, m, sqlstate, "", __FILE__, __LINE__)
#define SET_SNOWFLAKE_STMT_ERROR(e, ec, m, sqlstate, uuid) set_snowflake_error(e, ec, m, sqlstate, uuid, __FILE__, __LINE__)

void STDCALL set_snowflake_error(SF_ERROR *error,
                                  SF_ERROR_CODE error_code,
                                  const char *msg,
                                  const char *sqlstate,
                                  const char *sfqid,
                                  const char *file,
                                  int line);
void STDCALL clear_snowflake_error(SF_ERROR *error);
void STDCALL copy_snowflake_error(SF_ERROR *dst, SF_ERROR *src);

#ifdef __cplusplus
}
#endif

#endif //PDO_SNOWFLAKE_ERROR_H
