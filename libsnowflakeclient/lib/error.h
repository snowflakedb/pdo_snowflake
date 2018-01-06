/*
 * Copyright (c) 2017-2018 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef SNOWFLAKE_ERROR_H
#define SNOWFLAKE_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_WIN32)
#define STDCALL
#else
#define STDCALL __stdcall
#endif

#include <snowflake/client.h>

#define SET_SNOWFLAKE_ERROR(e, ec, m, sqlstate) set_snowflake_error(e, ec, m, sqlstate, "", __FILE__, __LINE__)
#define SET_SNOWFLAKE_STMT_ERROR(e, ec, m, sqlstate, uuid) set_snowflake_error(e, ec, m, sqlstate, uuid, __FILE__, __LINE__)

void STDCALL set_snowflake_error(SF_ERROR *error,
                                 SF_STATUS error_code,
                                 const char *msg,
                                 const char *sqlstate,
                                 const char *sfqid,
                                 const char *file,
                                 int line);

void STDCALL clear_snowflake_error(SF_ERROR *error);

void STDCALL copy_snowflake_error(SF_ERROR *dst, SF_ERROR *src);

#define ERR_MSG_ACCOUNT_PARAMETER_IS_MISSING "account parameter is missing"
#define ERR_MSG_USER_PARAMETER_IS_MISSING "user parameter is missing"
#define ERR_MSG_PASSWORD_PARAMETER_IS_MISSING "password parameter is missing"
#define ERR_MSG_CONNECTION_ALREADY_EXISTS "Connection already exists."

#ifdef __cplusplus
}
#endif

#endif //SNOWFLAKE_ERROR_H
