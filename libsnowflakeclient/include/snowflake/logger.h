/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */
/*
 * Copyright (c) 2017-2018 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef SNOWFLAKE_LOGGER_H
#define SNOWFLAKE_LOGGER_H

#include <stdio.h>
#include <stdarg.h>

typedef void (*log_LockFn)(void *udata, int lock);

typedef enum SF_LOG_LEVEL {
    SF_LOG_TRACE,
    SF_LOG_DEBUG,
    SF_LOG_INFO,
    SF_LOG_WARN,
    SF_LOG_ERROR,
    SF_LOG_FATAL
} SF_LOG_LEVEL;

#define log_trace(...) log_log(SF_LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(SF_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(SF_LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(SF_LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(SF_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(SF_LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

void log_set_udata(void *udata);

void log_set_lock(log_LockFn fn);

void log_set_fp(FILE *fp);

void log_set_level(int level);

void log_set_quiet(int enable);

void log_log(int level, const char *file, int line, const char *fmt, ...);

#endif /* SNOWFLAKE_LOGGER_H */
