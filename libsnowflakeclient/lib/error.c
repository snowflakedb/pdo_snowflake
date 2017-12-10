/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <string.h>
#include "error.h"
#include "snowflake_memory.h"

void STDCALL set_snowflake_error(SF_ERROR *error,
                                  SF_ERROR_CODE error_code,
                                  const char *msg,
                                  const char *sfqid,
                                  const char *file,
                                  int line) {
    // NULL error passed in. Should never happen
    if (!error) {
        return;
    }

    strncpy(error->sfqid, sfqid, UUID4_LEN);
    // Null terminate
    if (error->sfqid[UUID4_LEN - 1] != '\0') {
        error->sfqid[UUID4_LEN - 1] = '\0';
    }
    error->error_code = error_code;
    error->msg = msg;
    error->file = file;
    error->line = line;
}

void STDCALL clear_snowflake_error(SF_ERROR *error) {
    error->error_code = SF_ERROR_NONE;
    error->msg = NULL;
    error->file = NULL;
    error->line = 0;
    memset(error->sfqid, 0, UUID4_LEN);
}
