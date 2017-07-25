//
// Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
//

#ifndef PDO_SNOWFLAKE_SNOWFLAKE_CLIENT_H
#define PDO_SNOWFLAKE_SNOWFLAKE_CLIENT_H

#ifdef  __cplusplus
extern "C" {
#endif

#if !defined(_WIN32)
#define STDCALL
#else
#define STDCALL __stdcall
#endif

#include "snowflake_client_version.h"

typedef struct snowflake_restful
{
  char host[1024];
  int port;
  char protocol[6];
  int login_timeout;
  int request_timeout;
  char authenticator[1024];
  char token[1024];
  char master_token[4096];
  int session_id;
} snowflake_restful;

#ifdef  __cplusplus
}
#endif

#endif //PDO_SNOWFLAKE_SNOWFLAKE_CLIENT_H
