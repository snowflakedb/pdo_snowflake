/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef SNOWFLAKE_CONSTANTS_H
#define SNOWFLAKE_CONSTANTS_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_WIN32)
#define STDCALL
#else
#define STDCALL __stdcall
#endif

#include <snowflake/basic_types.h>

extern sf_bool DISABLE_VERIFY_PEER;
extern char *CA_BUNDLE_FILE;
extern int32 SSL_VERSION;
extern sf_bool DEBUG;

#ifdef __cplusplus
}
#endif

#endif //SNOWFLAKE_CONSTANTS_H
