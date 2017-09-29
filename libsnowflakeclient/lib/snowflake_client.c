/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <stddef.h>
#include "../include/snowflake_client.h"

SNOWFLAKE *STDCALL snowflake_init()
{
  return NULL;
}

void STDCALL snowflake_term(SNOWFLAKE *sf)
{
  // TODO
}

SNOWFLAKE_STATUS STDCALL snowflake_connect(SNOWFLAKE *sf)
{
  return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_close(SNOWFLAKE *sf)
{
  return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_set_attr(
    SNOWFLAKE *sf, SNOWFLAKE_ATTRIBUTE type, const void *value)
{
  return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STMT *STDCALL snowflake_stmt(SNOWFLAKE *sf)
{
  return NULL;
}

SNOWFLAKE_STATUS STDCALL snowflake_bind_param(
    SNOWFLAKE_STMT *sfstmt, SNOWFLAKE_INPUT *sfbind)
{
  return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_bind_result(
    SNOWFLAKE_STMT *sfstmt, SNOWFLAKE_OUTPUT *sfbind_array)
{
  return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_query(
    SNOWFLAKE_STMT *sfstmt, const char *command)
{
  return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_fetch(SNOWFLAKE_STMT *sfres)
{
  return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_stmt_close(SNOWFLAKE_STMT *sfres)
{
  return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_trans_begin(SNOWFLAKE *sf)
{
  return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_trans_commit(SNOWFLAKE *sf)
{
  return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_trans_rollback(SNOWFLAKE *sf)
{
  return SF_STATUS_SUCCESS;
}

int64 STDCALL snowflake_affected_rows(SNOWFLAKE_STMT *sfstmt)
{
  return 0;
}

SNOWFLAKE_STATUS STDCALL snowflake_prepare(
    SNOWFLAKE_STMT *sfstmt, const char *command)
{
  return SF_STATUS_SUCCESS;
}

SNOWFLAKE_STATUS STDCALL snowflake_execute(SNOWFLAKE_STMT *sfstmt)
{
  return SF_STATUS_SUCCESS;
}

SNOWFLAKE_ERROR *STDCALL snowflake_error(SNOWFLAKE_STMT *sfstmt)
{
  return NULL;
}

uint64 STDCALL snowflake_num_rows(SNOWFLAKE_STMT *sfres)
{
  return (uint64) 1;
}

const char *STDCALL snowflake_sfqid(SNOWFLAKE_STMT *sfstmt)
{
  return "abcdef-abdef";
}
