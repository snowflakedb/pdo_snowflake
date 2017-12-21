/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef SNOWFLAKE_SNOWFLAKE_CLIENT_INT_H
#define SNOWFLAKE_SNOWFLAKE_CLIENT_INT_H

#include "cJSON.h"
#include "arraylist.h"

#define HEADER_SNOWFLAKE_TOKEN_FORMAT "Authorization: Snowflake Token=\"%s\""
#define HEADER_CONTENT_TYPE_APPLICATION_JSON "Content-Type: application/json"
#define HEADER_ACCEPT_TYPE_APPLICATION_SNOWFLAKE "accept: application/snowflake"
#define HEADER_C_API_USER_AGENT "User-Agent: c_api/0.1"

#define DEFAULT_SNOWFLAKE_BASE_URL "snowflakecomputing.com"
#define DEFAULT_SNOWFLAKE_REQUEST_TIMEOUT 60

#define SESSION_URL "/session/v1/login-request?"
#define QUERY_URL "/queries/v1/query-request?"
#define RENEW_SESSION_URL "/session/token-request?"
#define DELETE_SESSION_URL "/session?"

#define SESSION_EXPIRE_CODE "390112"
#define QUERY_IN_PROGRESS_CODE "333333"
#define QUERY_IN_PROGRESS_ASYNC_CODE "333334"

#define REQUEST_TYPE_RENEW "RENEW"
#define REQUEST_TYPE_CLONE "CLONE"
#define REQUEST_TYPE_ISSUE "ISSUE"

int uuid4_generate(char *dst);

#endif //SNOWFLAKE_SNOWFLAKE_CLIENT_INT_H
