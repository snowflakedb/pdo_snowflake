/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <string.h>
#include "error.h"
#include "snowflake_memory.h"

void *STDCALL set_snowflake_error(SNOWFLAKE_ERROR *error,
                                  SNOWFLAKE_ERROR_CODE error_code,
                                  const char *msg,
                                  const char *sfqid,
                                  const char *file,
                                  int line) {
    size_t sfqid_size = 1;
    // NULL error passed in. Should never happen
    if (!error) {
        return NULL;
    }

    // Allocate enough memory for sfqid
    sfqid_size += strlen(sfqid);
    error->sfqid = (char *) SF_CALLOC(1, sfqid_size);
    // If there is an OOM error from trying to create a string for sfqid, then skip copying and just set original error
    if (error->sfqid) {
        strncpy(error->sfqid, sfqid, sfqid_size);
    }

    error->error_code = error_code;
    error->msg = msg;
    error->file = file;
    error->line = line;
}

void *STDCALL clear_snowflake_error(SNOWFLAKE_ERROR *error) {
    if (error) {
        SF_FREE(error->sfqid);
    }

}
