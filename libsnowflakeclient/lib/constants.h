/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef PDO_SNOWFLAKE_CONSTANTS_H
#define PDO_SNOWFLAKE_CONSTANTS_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_WIN32)
#define STDCALL
#else
#define STDCALL __stdcall
#endif

#include <basic_types.h>

extern sf_bool DISABLE_VERIFY_PEER;
extern char *CA_BUNDLE_FILE;
extern int32 SSL_VERSION;

#ifdef __cplusplus
}
#endif

#endif //PDO_SNOWFLAKE_CONSTANTS_H
