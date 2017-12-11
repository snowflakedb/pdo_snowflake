/*
 * Copyright (c) 2017 Snowflake Computing, Inc. All rights reserved.
 */

#include <string.h>
#include <log.h>
#include "error.h"
#include "snowflake_memory.h"

void STDCALL set_snowflake_error(SF_ERROR *error,
                                  SF_ERROR_CODE error_code,
                                  const char *msg,
                                  const char *sqlstate,
                                  const char *sfqid,
                                  const char *file,
                                  int line) {
    size_t msglen = strlen(msg);
    // NULL error passed in. Should never happen
    if (!error) {
        return;
    }

    error->error_code = error_code;
    strncpy(error->sfqid, sfqid, UUID4_LEN);
    // Null terminate
    if (error->sfqid[UUID4_LEN - 1] != '\0') {
        error->sfqid[UUID4_LEN - 1] = '\0';
    }

    if (sqlstate != NULL) {
        /* set SQLState if specified */
        strncpy(error->sqlstate, sqlstate, sizeof(error->sqlstate));
        error->sqlstate[sizeof(error->sqlstate)-1] = '\0';
    }

    if (error->msg != NULL) {
        /* free message buffer first if already exists */
        SF_FREE(error->msg);
    }

    /* allocate new memory */
    error->msg = SF_CALLOC(msglen + 1, sizeof(char));
    strncpy(error->msg, msg, msglen);
    error->msg[msglen] = '\0';

    error->file = (char*)file;
    error->line = line;
}

void STDCALL clear_snowflake_error(SF_ERROR *error) {
    if (strncmp(error->sqlstate, SF_SQLSTATE_NO_ERROR, sizeof(SF_SQLSTATE_NO_ERROR))) {
        /* Error already set */
        SF_FREE(error->msg);
        strcpy(error->sqlstate, SF_SQLSTATE_NO_ERROR);
    }
    error->error_code = SF_ERROR_NONE;
    error->msg = NULL;
    error->file = NULL;
    error->line = 0;
    memset(error->sfqid, 0, UUID4_LEN);
}
